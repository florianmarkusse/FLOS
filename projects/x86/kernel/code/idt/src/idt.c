#include "abstraction/interrupts.h"

#include "abstraction/log.h"
#include "abstraction/memory/virtual/map.h"
#include "abstraction/thread.h"
#include "shared/enum.h"
#include "shared/log.h"
#include "shared/maths.h"
#include "shared/memory/converter.h"
#include "shared/memory/management/management.h"
#include "shared/memory/management/page.h"
#include "shared/memory/sizes.h"
#include "shared/types/numeric.h"
#include "x86/configuration/cpu.h"
#include "x86/fault.h"
#include "x86/kernel/idt.h"
#include "x86/memory/definitions.h"

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void isr32();
extern void isr33();
extern void isr34();
extern void isr35();
extern void isr36();
extern void isr37();
extern void isr38();
extern void isr39();
extern void isr40();
extern void isr41();
extern void isr42();
extern void isr43();
extern void isr44();
extern void isr45();
extern void isr46();
extern void isr47();
extern void isr48();
extern void isr49();
extern void isr50();
extern void isr51();
extern void isr52();
extern void isr53();
extern void isr54();
extern void isr55();
extern void isr56();
extern void isr57();
extern void isr58();
extern void isr59();
extern void isr60();
extern void isr61();
extern void isr62();
extern void isr63();
extern void isr64();
extern void isr65();
extern void isr66();
extern void isr67();
extern void isr68();
extern void isr69();
extern void isr70();
extern void isr71();
extern void isr72();
extern void isr73();
extern void isr74();
extern void isr75();
extern void isr76();
extern void isr77();
extern void isr78();
extern void isr79();
extern void isr80();
extern void isr81();
extern void isr82();
extern void isr83();
extern void isr84();
extern void isr85();
extern void isr86();
extern void isr87();
extern void isr88();
extern void isr89();
extern void isr90();
extern void isr91();
extern void isr92();
extern void isr93();
extern void isr94();
extern void isr95();
extern void isr96();
extern void isr97();
extern void isr98();
extern void isr99();
extern void isr100();
extern void isr101();
extern void isr102();
extern void isr103();
extern void isr104();
extern void isr105();
extern void isr106();
extern void isr107();
extern void isr108();
extern void isr109();
extern void isr110();
extern void isr111();
extern void isr112();
extern void isr113();
extern void isr114();
extern void isr115();
extern void isr116();
extern void isr117();
extern void isr118();
extern void isr119();
extern void isr120();
extern void isr121();
extern void isr122();
extern void isr123();
extern void isr124();
extern void isr125();
extern void isr126();
extern void isr127();
extern void isr128();
extern void isr129();
extern void isr130();
extern void isr131();
extern void isr132();
extern void isr133();
extern void isr134();
extern void isr135();
extern void isr136();
extern void isr137();
extern void isr138();
extern void isr139();
extern void isr140();
extern void isr141();
extern void isr142();
extern void isr143();
extern void isr144();
extern void isr145();
extern void isr146();
extern void isr147();
extern void isr148();
extern void isr149();
extern void isr150();
extern void isr151();
extern void isr152();
extern void isr153();
extern void isr154();
extern void isr155();
extern void isr156();
extern void isr157();
extern void isr158();
extern void isr159();
extern void isr160();
extern void isr161();
extern void isr162();
extern void isr163();
extern void isr164();
extern void isr165();
extern void isr166();
extern void isr167();
extern void isr168();
extern void isr169();
extern void isr170();
extern void isr171();
extern void isr172();
extern void isr173();
extern void isr174();
extern void isr175();
extern void isr176();
extern void isr177();
extern void isr178();
extern void isr179();
extern void isr180();
extern void isr181();
extern void isr182();
extern void isr183();
extern void isr184();
extern void isr185();
extern void isr186();
extern void isr187();
extern void isr188();
extern void isr189();
extern void isr190();
extern void isr191();
extern void isr192();
extern void isr193();
extern void isr194();
extern void isr195();
extern void isr196();
extern void isr197();
extern void isr198();
extern void isr199();
extern void isr200();
extern void isr201();
extern void isr202();
extern void isr203();
extern void isr204();
extern void isr205();
extern void isr206();
extern void isr207();
extern void isr208();
extern void isr209();
extern void isr210();
extern void isr211();
extern void isr212();
extern void isr213();
extern void isr214();
extern void isr215();
extern void isr216();
extern void isr217();
extern void isr218();
extern void isr219();
extern void isr220();
extern void isr221();
extern void isr222();
extern void isr223();
extern void isr224();
extern void isr225();
extern void isr226();
extern void isr227();
extern void isr228();
extern void isr229();
extern void isr230();
extern void isr231();
extern void isr232();
extern void isr233();
extern void isr234();
extern void isr235();
extern void isr236();
extern void isr237();
extern void isr238();
extern void isr239();
extern void isr240();
extern void isr241();
extern void isr242();
extern void isr243();
extern void isr244();
extern void isr245();
extern void isr246();
extern void isr247();
extern void isr248();
extern void isr249();
extern void isr250();
extern void isr251();
extern void isr252();
extern void isr253();
extern void isr254();
extern void isr255();

extern void asm_lidt(void *);

typedef struct __attribute__((packed)) {
    U16 limit;
    U64 base;
} idt_ptr;

typedef struct __attribute__((packed)) {
    U16 offset_1; // offset bits 0..15
    U16 selector; // a code segment selector in GDT or LDT
    U8 ist; // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.
    U8 type_attributes; // gate type, dpl, and p fields
    U16 offset_2;       // offset bits 16..31
    U32 offset_3;       // offset bits 32..63
    U32 zero;           // reserved
} InterruptDescriptor;

static InterruptDescriptor idt[256];

#define GATE_TYPE_ENUM(VARIANT)                                                \
    VARIANT(INTERRUPT_GATE)                                                    \
    VARIANT(TRAP_GATE)

typedef enum { GATE_TYPE_ENUM(ENUM_STANDARD_VARIANT) } GateType;
static constexpr auto GATE_TYPE_COUNT = (0 GATE_TYPE_ENUM(PLUS_ONE));

U8 gateFlags[GATE_TYPE_COUNT] = {0x8E, 0x8F};

static void idt_set_gate(U8 num, U64 base, IST ist) {
    idt[num].offset_1 = (base & 0xFFFF);
    idt[num].offset_2 = (base >> 16) & 0xFFFF;
    idt[num].offset_3 = (base >> 32) & 0xFFFFFFFF;

    // Referring to the GDT segment here
    idt[num].selector = 0x08;

    idt[num].type_attributes = gateFlags[INTERRUPT_GATE];

    // NOTE: ist is an enum from 0...6, so increment by 1 as 0 is the setting
    // for no IST, just putting the IST straight onto the original stack.
    idt[num].ist = ist + 1;
    idt[num].zero = 0;
}

void interruptsInit() {
    idt_ptr idtp;
    idtp.limit = (sizeof(InterruptDescriptor) * 256) - 1;
    idtp.base = (U64)&idt;

    idt_set_gate(0, (U64)isr0, MASKABLE_IST);
    idt_set_gate(1, (U64)isr1, MASKABLE_IST);
    idt_set_gate(2, (U64)isr2, NON_MASKABLE_INTERRUPT_IST);
    idt_set_gate(3, (U64)isr3, MASKABLE_IST);
    idt_set_gate(4, (U64)isr4, MASKABLE_IST);
    idt_set_gate(5, (U64)isr5, MASKABLE_IST);
    idt_set_gate(6, (U64)isr6, MASKABLE_IST);
    idt_set_gate(7, (U64)isr7, MASKABLE_IST);
    idt_set_gate(8, (U64)isr8, DOUBLE_FAULT_IST);
    idt_set_gate(9, (U64)isr9, MASKABLE_IST);
    idt_set_gate(10, (U64)isr10, MASKABLE_IST);
    idt_set_gate(11, (U64)isr11, MASKABLE_IST);
    idt_set_gate(12, (U64)isr12, MASKABLE_IST);
    idt_set_gate(13, (U64)isr13, MASKABLE_IST);
    idt_set_gate(14, (U64)isr14, FAULT_PAGE_IST);
    idt_set_gate(15, (U64)isr15, MASKABLE_IST);
    idt_set_gate(16, (U64)isr16, MASKABLE_IST);
    idt_set_gate(17, (U64)isr17, MASKABLE_IST);
    idt_set_gate(18, (U64)isr18, MACHINE_CHECK_IST);
    idt_set_gate(19, (U64)isr19, MASKABLE_IST);
    idt_set_gate(20, (U64)isr20, MASKABLE_IST);
    idt_set_gate(21, (U64)isr21, MASKABLE_IST);
    idt_set_gate(22, (U64)isr22, MASKABLE_IST);
    idt_set_gate(23, (U64)isr23, MASKABLE_IST);
    idt_set_gate(24, (U64)isr24, MASKABLE_IST);
    idt_set_gate(25, (U64)isr25, MASKABLE_IST);
    idt_set_gate(26, (U64)isr26, MASKABLE_IST);
    idt_set_gate(27, (U64)isr27, MASKABLE_IST);
    idt_set_gate(28, (U64)isr28, MASKABLE_IST);
    idt_set_gate(29, (U64)isr29, MASKABLE_IST);
    idt_set_gate(30, (U64)isr30, MASKABLE_IST);
    idt_set_gate(31, (U64)isr31, MASKABLE_IST);
    idt_set_gate(32, (U64)isr32, MASKABLE_IST);
    idt_set_gate(33, (U64)isr33, MASKABLE_IST);
    idt_set_gate(34, (U64)isr34, MASKABLE_IST);
    idt_set_gate(35, (U64)isr35, MASKABLE_IST);
    idt_set_gate(36, (U64)isr36, MASKABLE_IST);
    idt_set_gate(37, (U64)isr37, MASKABLE_IST);
    idt_set_gate(38, (U64)isr38, MASKABLE_IST);
    idt_set_gate(39, (U64)isr39, MASKABLE_IST);
    idt_set_gate(40, (U64)isr40, MASKABLE_IST);
    idt_set_gate(41, (U64)isr41, MASKABLE_IST);
    idt_set_gate(42, (U64)isr42, MASKABLE_IST);
    idt_set_gate(43, (U64)isr43, MASKABLE_IST);
    idt_set_gate(44, (U64)isr44, MASKABLE_IST);
    idt_set_gate(45, (U64)isr45, MASKABLE_IST);
    idt_set_gate(46, (U64)isr46, MASKABLE_IST);
    idt_set_gate(47, (U64)isr47, MASKABLE_IST);
    idt_set_gate(48, (U64)isr48, MASKABLE_IST);
    idt_set_gate(49, (U64)isr49, MASKABLE_IST);
    idt_set_gate(50, (U64)isr50, MASKABLE_IST);
    idt_set_gate(51, (U64)isr51, MASKABLE_IST);
    idt_set_gate(52, (U64)isr52, MASKABLE_IST);
    idt_set_gate(53, (U64)isr53, MASKABLE_IST);
    idt_set_gate(54, (U64)isr54, MASKABLE_IST);
    idt_set_gate(55, (U64)isr55, MASKABLE_IST);
    idt_set_gate(56, (U64)isr56, MASKABLE_IST);
    idt_set_gate(57, (U64)isr57, MASKABLE_IST);
    idt_set_gate(58, (U64)isr58, MASKABLE_IST);
    idt_set_gate(59, (U64)isr59, MASKABLE_IST);
    idt_set_gate(60, (U64)isr60, MASKABLE_IST);
    idt_set_gate(61, (U64)isr61, MASKABLE_IST);
    idt_set_gate(62, (U64)isr62, MASKABLE_IST);
    idt_set_gate(63, (U64)isr63, MASKABLE_IST);
    idt_set_gate(64, (U64)isr64, MASKABLE_IST);
    idt_set_gate(65, (U64)isr65, MASKABLE_IST);
    idt_set_gate(66, (U64)isr66, MASKABLE_IST);
    idt_set_gate(67, (U64)isr67, MASKABLE_IST);
    idt_set_gate(68, (U64)isr68, MASKABLE_IST);
    idt_set_gate(69, (U64)isr69, MASKABLE_IST);
    idt_set_gate(70, (U64)isr70, MASKABLE_IST);
    idt_set_gate(71, (U64)isr71, MASKABLE_IST);
    idt_set_gate(72, (U64)isr72, MASKABLE_IST);
    idt_set_gate(73, (U64)isr73, MASKABLE_IST);
    idt_set_gate(74, (U64)isr74, MASKABLE_IST);
    idt_set_gate(75, (U64)isr75, MASKABLE_IST);
    idt_set_gate(76, (U64)isr76, MASKABLE_IST);
    idt_set_gate(77, (U64)isr77, MASKABLE_IST);
    idt_set_gate(78, (U64)isr78, MASKABLE_IST);
    idt_set_gate(79, (U64)isr79, MASKABLE_IST);
    idt_set_gate(80, (U64)isr80, MASKABLE_IST);
    idt_set_gate(81, (U64)isr81, MASKABLE_IST);
    idt_set_gate(82, (U64)isr82, MASKABLE_IST);
    idt_set_gate(83, (U64)isr83, MASKABLE_IST);
    idt_set_gate(84, (U64)isr84, MASKABLE_IST);
    idt_set_gate(85, (U64)isr85, MASKABLE_IST);
    idt_set_gate(86, (U64)isr86, MASKABLE_IST);
    idt_set_gate(87, (U64)isr87, MASKABLE_IST);
    idt_set_gate(88, (U64)isr88, MASKABLE_IST);
    idt_set_gate(89, (U64)isr89, MASKABLE_IST);
    idt_set_gate(90, (U64)isr90, MASKABLE_IST);
    idt_set_gate(91, (U64)isr91, MASKABLE_IST);
    idt_set_gate(92, (U64)isr92, MASKABLE_IST);
    idt_set_gate(93, (U64)isr93, MASKABLE_IST);
    idt_set_gate(94, (U64)isr94, MASKABLE_IST);
    idt_set_gate(95, (U64)isr95, MASKABLE_IST);
    idt_set_gate(96, (U64)isr96, MASKABLE_IST);
    idt_set_gate(97, (U64)isr97, MASKABLE_IST);
    idt_set_gate(98, (U64)isr98, MASKABLE_IST);
    idt_set_gate(99, (U64)isr99, MASKABLE_IST);
    idt_set_gate(100, (U64)isr100, MASKABLE_IST);
    idt_set_gate(101, (U64)isr101, MASKABLE_IST);
    idt_set_gate(102, (U64)isr102, MASKABLE_IST);
    idt_set_gate(103, (U64)isr103, MASKABLE_IST);
    idt_set_gate(104, (U64)isr104, MASKABLE_IST);
    idt_set_gate(105, (U64)isr105, MASKABLE_IST);
    idt_set_gate(106, (U64)isr106, MASKABLE_IST);
    idt_set_gate(107, (U64)isr107, MASKABLE_IST);
    idt_set_gate(108, (U64)isr108, MASKABLE_IST);
    idt_set_gate(109, (U64)isr109, MASKABLE_IST);
    idt_set_gate(110, (U64)isr110, MASKABLE_IST);
    idt_set_gate(111, (U64)isr111, MASKABLE_IST);
    idt_set_gate(112, (U64)isr112, MASKABLE_IST);
    idt_set_gate(113, (U64)isr113, MASKABLE_IST);
    idt_set_gate(114, (U64)isr114, MASKABLE_IST);
    idt_set_gate(115, (U64)isr115, MASKABLE_IST);
    idt_set_gate(116, (U64)isr116, MASKABLE_IST);
    idt_set_gate(117, (U64)isr117, MASKABLE_IST);
    idt_set_gate(118, (U64)isr118, MASKABLE_IST);
    idt_set_gate(119, (U64)isr119, MASKABLE_IST);
    idt_set_gate(120, (U64)isr120, MASKABLE_IST);
    idt_set_gate(121, (U64)isr121, MASKABLE_IST);
    idt_set_gate(122, (U64)isr122, MASKABLE_IST);
    idt_set_gate(123, (U64)isr123, MASKABLE_IST);
    idt_set_gate(124, (U64)isr124, MASKABLE_IST);
    idt_set_gate(125, (U64)isr125, MASKABLE_IST);
    idt_set_gate(126, (U64)isr126, MASKABLE_IST);
    idt_set_gate(127, (U64)isr127, MASKABLE_IST);
    idt_set_gate(128, (U64)isr128, MASKABLE_IST);
    idt_set_gate(129, (U64)isr129, MASKABLE_IST);
    idt_set_gate(130, (U64)isr130, MASKABLE_IST);
    idt_set_gate(131, (U64)isr131, MASKABLE_IST);
    idt_set_gate(132, (U64)isr132, MASKABLE_IST);
    idt_set_gate(133, (U64)isr133, MASKABLE_IST);
    idt_set_gate(134, (U64)isr134, MASKABLE_IST);
    idt_set_gate(135, (U64)isr135, MASKABLE_IST);
    idt_set_gate(136, (U64)isr136, MASKABLE_IST);
    idt_set_gate(137, (U64)isr137, MASKABLE_IST);
    idt_set_gate(138, (U64)isr138, MASKABLE_IST);
    idt_set_gate(139, (U64)isr139, MASKABLE_IST);
    idt_set_gate(140, (U64)isr140, MASKABLE_IST);
    idt_set_gate(141, (U64)isr141, MASKABLE_IST);
    idt_set_gate(142, (U64)isr142, MASKABLE_IST);
    idt_set_gate(143, (U64)isr143, MASKABLE_IST);
    idt_set_gate(144, (U64)isr144, MASKABLE_IST);
    idt_set_gate(145, (U64)isr145, MASKABLE_IST);
    idt_set_gate(146, (U64)isr146, MASKABLE_IST);
    idt_set_gate(147, (U64)isr147, MASKABLE_IST);
    idt_set_gate(148, (U64)isr148, MASKABLE_IST);
    idt_set_gate(149, (U64)isr149, MASKABLE_IST);
    idt_set_gate(150, (U64)isr150, MASKABLE_IST);
    idt_set_gate(151, (U64)isr151, MASKABLE_IST);
    idt_set_gate(152, (U64)isr152, MASKABLE_IST);
    idt_set_gate(153, (U64)isr153, MASKABLE_IST);
    idt_set_gate(154, (U64)isr154, MASKABLE_IST);
    idt_set_gate(155, (U64)isr155, MASKABLE_IST);
    idt_set_gate(156, (U64)isr156, MASKABLE_IST);
    idt_set_gate(157, (U64)isr157, MASKABLE_IST);
    idt_set_gate(158, (U64)isr158, MASKABLE_IST);
    idt_set_gate(159, (U64)isr159, MASKABLE_IST);
    idt_set_gate(160, (U64)isr160, MASKABLE_IST);
    idt_set_gate(161, (U64)isr161, MASKABLE_IST);
    idt_set_gate(162, (U64)isr162, MASKABLE_IST);
    idt_set_gate(163, (U64)isr163, MASKABLE_IST);
    idt_set_gate(164, (U64)isr164, MASKABLE_IST);
    idt_set_gate(165, (U64)isr165, MASKABLE_IST);
    idt_set_gate(166, (U64)isr166, MASKABLE_IST);
    idt_set_gate(167, (U64)isr167, MASKABLE_IST);
    idt_set_gate(168, (U64)isr168, MASKABLE_IST);
    idt_set_gate(169, (U64)isr169, MASKABLE_IST);
    idt_set_gate(170, (U64)isr170, MASKABLE_IST);
    idt_set_gate(171, (U64)isr171, MASKABLE_IST);
    idt_set_gate(172, (U64)isr172, MASKABLE_IST);
    idt_set_gate(173, (U64)isr173, MASKABLE_IST);
    idt_set_gate(174, (U64)isr174, MASKABLE_IST);
    idt_set_gate(175, (U64)isr175, MASKABLE_IST);
    idt_set_gate(176, (U64)isr176, MASKABLE_IST);
    idt_set_gate(177, (U64)isr177, MASKABLE_IST);
    idt_set_gate(178, (U64)isr178, MASKABLE_IST);
    idt_set_gate(179, (U64)isr179, MASKABLE_IST);
    idt_set_gate(180, (U64)isr180, MASKABLE_IST);
    idt_set_gate(181, (U64)isr181, MASKABLE_IST);
    idt_set_gate(182, (U64)isr182, MASKABLE_IST);
    idt_set_gate(183, (U64)isr183, MASKABLE_IST);
    idt_set_gate(184, (U64)isr184, MASKABLE_IST);
    idt_set_gate(185, (U64)isr185, MASKABLE_IST);
    idt_set_gate(186, (U64)isr186, MASKABLE_IST);
    idt_set_gate(187, (U64)isr187, MASKABLE_IST);
    idt_set_gate(188, (U64)isr188, MASKABLE_IST);
    idt_set_gate(189, (U64)isr189, MASKABLE_IST);
    idt_set_gate(190, (U64)isr190, MASKABLE_IST);
    idt_set_gate(191, (U64)isr191, MASKABLE_IST);
    idt_set_gate(192, (U64)isr192, MASKABLE_IST);
    idt_set_gate(193, (U64)isr193, MASKABLE_IST);
    idt_set_gate(194, (U64)isr194, MASKABLE_IST);
    idt_set_gate(195, (U64)isr195, MASKABLE_IST);
    idt_set_gate(196, (U64)isr196, MASKABLE_IST);
    idt_set_gate(197, (U64)isr197, MASKABLE_IST);
    idt_set_gate(198, (U64)isr198, MASKABLE_IST);
    idt_set_gate(199, (U64)isr199, MASKABLE_IST);
    idt_set_gate(200, (U64)isr200, MASKABLE_IST);
    idt_set_gate(201, (U64)isr201, MASKABLE_IST);
    idt_set_gate(202, (U64)isr202, MASKABLE_IST);
    idt_set_gate(203, (U64)isr203, MASKABLE_IST);
    idt_set_gate(204, (U64)isr204, MASKABLE_IST);
    idt_set_gate(205, (U64)isr205, MASKABLE_IST);
    idt_set_gate(206, (U64)isr206, MASKABLE_IST);
    idt_set_gate(207, (U64)isr207, MASKABLE_IST);
    idt_set_gate(208, (U64)isr208, MASKABLE_IST);
    idt_set_gate(209, (U64)isr209, MASKABLE_IST);
    idt_set_gate(210, (U64)isr210, MASKABLE_IST);
    idt_set_gate(211, (U64)isr211, MASKABLE_IST);
    idt_set_gate(212, (U64)isr212, MASKABLE_IST);
    idt_set_gate(213, (U64)isr213, MASKABLE_IST);
    idt_set_gate(214, (U64)isr214, MASKABLE_IST);
    idt_set_gate(215, (U64)isr215, MASKABLE_IST);
    idt_set_gate(216, (U64)isr216, MASKABLE_IST);
    idt_set_gate(217, (U64)isr217, MASKABLE_IST);
    idt_set_gate(218, (U64)isr218, MASKABLE_IST);
    idt_set_gate(219, (U64)isr219, MASKABLE_IST);
    idt_set_gate(220, (U64)isr220, MASKABLE_IST);
    idt_set_gate(221, (U64)isr221, MASKABLE_IST);
    idt_set_gate(222, (U64)isr222, MASKABLE_IST);
    idt_set_gate(223, (U64)isr223, MASKABLE_IST);
    idt_set_gate(224, (U64)isr224, MASKABLE_IST);
    idt_set_gate(225, (U64)isr225, MASKABLE_IST);
    idt_set_gate(226, (U64)isr226, MASKABLE_IST);
    idt_set_gate(227, (U64)isr227, MASKABLE_IST);
    idt_set_gate(228, (U64)isr228, MASKABLE_IST);
    idt_set_gate(229, (U64)isr229, MASKABLE_IST);
    idt_set_gate(230, (U64)isr230, MASKABLE_IST);
    idt_set_gate(231, (U64)isr231, MASKABLE_IST);
    idt_set_gate(232, (U64)isr232, MASKABLE_IST);
    idt_set_gate(233, (U64)isr233, MASKABLE_IST);
    idt_set_gate(234, (U64)isr234, MASKABLE_IST);
    idt_set_gate(235, (U64)isr235, MASKABLE_IST);
    idt_set_gate(236, (U64)isr236, MASKABLE_IST);
    idt_set_gate(237, (U64)isr237, MASKABLE_IST);
    idt_set_gate(238, (U64)isr238, MASKABLE_IST);
    idt_set_gate(239, (U64)isr239, MASKABLE_IST);
    idt_set_gate(240, (U64)isr240, MASKABLE_IST);
    idt_set_gate(241, (U64)isr241, MASKABLE_IST);
    idt_set_gate(242, (U64)isr242, MASKABLE_IST);
    idt_set_gate(243, (U64)isr243, MASKABLE_IST);
    idt_set_gate(244, (U64)isr244, MASKABLE_IST);
    idt_set_gate(245, (U64)isr245, MASKABLE_IST);
    idt_set_gate(246, (U64)isr246, MASKABLE_IST);
    idt_set_gate(247, (U64)isr247, MASKABLE_IST);
    idt_set_gate(248, (U64)isr248, MASKABLE_IST);
    idt_set_gate(249, (U64)isr249, MASKABLE_IST);
    idt_set_gate(250, (U64)isr250, MASKABLE_IST);
    idt_set_gate(251, (U64)isr251, MASKABLE_IST);
    idt_set_gate(252, (U64)isr252, MASKABLE_IST);
    idt_set_gate(253, (U64)isr253, MASKABLE_IST);
    idt_set_gate(254, (U64)isr254, MASKABLE_IST);
    idt_set_gate(255, (U64)isr255, MASKABLE_IST);

    asm_lidt(&idtp);
}

void faultTrigger(Fault fault) {
    switch (fault) {
    case FAULT_DIVIDE_ERROR:
        asm volatile("int $0x00" :::);
        break;
    case FAULT_DEBUG:
        asm volatile("int $0x01" :::);
        break;
    case FAULT_NMI:
        asm volatile("int $0x02" :::);
        break;
    case FAULT_BREAKPOINT:
        asm volatile("int $0x03" :::);
        break;
    case FAULT_OVERFLOW:
        asm volatile("int $0x04" :::);
        break;
    case FAULT_BOUND_RANGE_EXCEED:
        asm volatile("int $0x05" :::);
        break;
    case FAULT_INVALID_OPCODE:
        asm volatile("int $0x06" :::);
        break;
    case FAULT_DEVICE_NOT_AVAILABLE:
        asm volatile("int $0x07" :::);
        break;
    case FAULT_DOUBLE_FAULT:
        asm volatile("int $0x08" :::);
        break;
    case FAULT_9_RESERVED:
        asm volatile("int $0x09" :::);
        break;
    case FAULT_INVALID_TSS:
        asm volatile("int $0x0A" :::);
        break;
    case FAULT_SEGMENT_NOT_PRESENT:
        asm volatile("int $0x0B" :::);
        break;
    case FAULT_STACK_FAULT:
        asm volatile("int $0x0C" :::);
        break;
    case FAULT_GENERAL_PROTECTION:
        asm volatile("int $0x0D" :::);
        break;
    case FAULT_PAGE_FAULT:
        asm volatile("int $0x0E" :::);
        break;
    case FAULT_15_RESERVED:
        asm volatile("int $0x0F" :::);
        break;
    case FAULT_FPU_ERROR:
        asm volatile("int $0x10" :::);
        break;
    case FAULT_ALIGNMENT_CHECK:
        asm volatile("int $0x11" :::);
        break;
    case FAULT_MACHINE_CHECK:
        asm volatile("int $0x12" :::);
        break;
    case FAULT_SIMD_FLOATING_POINT:
        asm volatile("int $0x13" :::);
        break;
    case FAULT_VIRTUALIZATION:
        asm volatile("int $0x14" :::);
        break;
    case FAULT_CONTROL_PROTECTION:
        asm volatile("int $0x15" :::);
        break;
    case FAULT_22_RESERVED:
        asm volatile("int $0x16" :::);
        break;
    case FAULT_23_RESERVED:
        asm volatile("int $0x17" :::);
        break;
    case FAULT_24_RESERVED:
        asm volatile("int $0x18" :::);
        break;
    case FAULT_25_RESERVED:
        asm volatile("int $0x19" :::);
        break;
    case FAULT_26_RESERVED:
        asm volatile("int $0x1A" :::);
        break;
    case FAULT_27_RESERVED:
        asm volatile("int $0x1B" :::);
        break;
    case FAULT_28_RESERVED:
        asm volatile("int $0x1C" :::);
        break;
    case FAULT_29_RESERVED:
        asm volatile("int $0x1D" :::);
        break;
    case FAULT_30_RESERVED:
        asm volatile("int $0x1E" :::);
        break;
    case FAULT_31_RESERVED:
        asm volatile("int $0x1F" :::);
        break;
    case FAULT_USER:
        asm volatile("int $0x20" :::);
        break;
    case FAULT_SYSCALL:
        asm volatile("int $0x21" :::);
        break;
    case FAULT_NO_MORE_PHYSICAL_MEMORY:
        asm volatile("int $0x22" :::);
        break;
    case FAULT_NO_MORE_VIRTUAL_MEMORY:
        asm volatile("int $0x23" :::);
        break;
    case FAULT_NO_MORE_VIRTUAL_MEMORY_MAPPER:
        asm volatile("int $0x24" :::);
        break;
    case FAULT_NO_MORE_BUFFER:
        asm volatile("int $0x25" :::);
        break;
    case FAULT_UNEXPECTED_FAILURE:
        asm volatile("int $0x26" :::);
        break;
    default:
        asm volatile("int $0xFF" :::);
        break;
    }

    __builtin_unreachable();
}
