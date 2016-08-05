#include <QDebug>
#include <sys/mman.h>
#include <cpuid.h>

#define NOP "0x0f,0x1f,0x44,0x00,0"

struct jump_entry {
    ulong code;
    ulong target;
    ulong key;
};

extern struct jump_entry __start___jump_table;
extern struct jump_entry __stop___jump_table;

struct static_key {
    int enabled;
    struct jump_entry *entries;
};

static inline __attribute__((always_inline)) bool asm_goto_basic(int state)
{
    asm goto (
        "btl %1, %0\n\t"        /* Selects the bit in a bit string (%0) at the bit-position designated by the bit offset (%0) */
        "jc %l[carry]"
        :                       /* No outputs. */
        : "r" (state), "r" (0)  /* Inputs: bit string, bit position */
        : "cc"                  /* clobbers: the "cc" clobber indicates that the assembler code modifies the flags register. */
        : carry);               /* List of goto labels */

    return false;

carry:
    return true;
}

static inline __attribute__((always_inline)) bool asm_goto_full(struct static_key *key, bool branch)
{
    asm goto("1:"
        ".byte 0x0f,0x1f,0x44,0x00,0 \n\t"
        ".pushsection __jump_table,  \"aw\" \n\t"
        ".balign 8 \n\t"
        ".quad 1b, %l[l_yes], %c0 + %c1 \n\t"
        ".popsection \n\t"
        :
        : "i" (key), "i" (branch)
        :
        : l_yes);
    asm ("");
    return false;
l_yes:
    return true;
}

static struct static_key key;

/*
$ readelf -x __jump_table 31-asm-goto

Hex dump of section '__jump_table':
  0x00601058 00084000 00000000 28084000 00000000 ..@.....(.@.....
  0x00601068 81106000 00000000                   ..`.....

$ nm 31-asm-goto | grep key
0000000000601080 b _ZL3key

00000000004007f0 <_Z3foov>:
  4007f0:       48 8d 3d 0d 01 00 00    lea    0x10d(%rip),%rdi        # 400904 <_IO_stdin_used+0x4>
  4007f7:       48 83 ec 08             sub    $0x8,%rsp
  4007fb:       e8 40 fd ff ff          callq  400540 <puts@plt>
  400800:       0f 1f 44 00 00          nopl   0x0(%rax,%rax,1)
  400805:       48 8d 3d 02 01 00 00    lea    0x102(%rip),%rdi        # 40090e <_IO_stdin_used+0xe>
  40080c:       e8 2f fd ff ff          callq  400540 <puts@plt>
  400811:       48 8d 3d 0a 01 00 00    lea    0x10a(%rip),%rdi        # 400922 <_IO_stdin_used+0x22>
  400818:       48 83 c4 08             add    $0x8,%rsp
  40081c:       e9 1f fd ff ff          jmpq   400540 <puts@plt>
  400821:       0f 1f 80 00 00 00 00    nopl   0x0(%rax)
  400828:       48 8d 3d fc 00 00 00    lea    0xfc(%rip),%rdi        # 40092b <_IO_stdin_used+0x2b>
  40082f:       e8 0c fd ff ff          callq  400540 <puts@plt>
  400834:       eb db                   jmp    400811 <_Z3foov+0x21>
  400836:       66 2e 0f 1f 84 00 00    nopw   %cs:0x0(%rax,%rax,1)
  40083d:       00 00 00
*/

void cpuid()
{
    int a, b, c, d;
    __cpuid(0, a, b, c, d);
}

void foo()
{
    printf("foo entry\n");
    if (asm_goto_full(&key, 1)) {
        printf("STATIC_KEY IS TRUE\n");
    } else {
        printf("STATIC_KEY IS FALSE\n");
    }
    printf("foo exit\n");
}

union jump_code_union {
    char code[5];
    struct {
        char jump;
        int offset;
    } __attribute__((packed));
};

void print_mem(const char *msg, uchar *ip)
{
    printf("%s [%p] (%02x %02x %02x %02x %02x)\n",
               msg, ip, ip[0], ip[1], ip[2], ip[3], ip[4]);
}

int main()
{
    /*
     * Display all static keys uses
     */
    printf("static key entries\n");
    struct jump_entry *entry = &__start___jump_table;
    for (; entry < &__stop___jump_table; entry++) {
        printf("entry: %p\n", entry);
        printf("0x%lx 0x%lx 0x%lx\n", entry->code, entry->target, entry->key);
    }
    fflush(stdout);

    /*
     * Let's use the first entry
     */
    entry = &__start___jump_table;

    /*
     * To change the code, the page must be writable
     */
    void *page = (void *) (entry->code & ((~0L) << 12));
    int ret = mprotect(page, 4096, PROT_READ | PROT_WRITE | PROT_EXEC);
    if (ret < 0) {
        perror("mprotect");
        return -1;
    }

    // First execution, static key should be false
    foo();

    union jump_code_union code;
    code.jump = 0xe9; // x86 JMP opcode
    code.offset = entry->target - (entry->code + 5); // relative offset, 4 bytes operand
    printf("jump relative: %x\n", code.offset);
    printf("jump absolute: %x\n", entry->code + code.offset + 5);
    print_mem("before", (uchar *)entry->code);

    /*
     * Change the code
     */
    memcpy((void *)entry->code, &code, 5);

    /*
     * CPUID is an instruction that forces the CPU
     * to throw speculative execution and fetch the
     * instructions again
     */
    cpuid();

    print_mem("after ", (uchar *)entry->code);

    // Second execution, static key should be true
    foo();

    return 0;
}

