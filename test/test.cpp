#define CATCH_CONFIG_MAIN
#include "../include/catch.hpp"

#include "../src/Utilities.hpp"

TEST_CASE("Emulator is constructed properly", "[Emulator][get_register]") {
    Emulator* vm = new Emulator(128);

    WORD program[5] = {0x01, 0x02, 0x03, 0x04, 0x0f};
    Emulator* vm2 = new Emulator(128, program, 5);

    SECTION("Retrieving any register will return 0") {
        for(int i = 0; i < 32; i++) {
            REQUIRE(vm->get_register(i) == 0);
        }
    }

    SECTION("Memory will be intialized to 0") {
        for(int i = 0; i < 128; i++) {
            REQUIRE(vm->load_byte(i) == 0);
        }
    }

    SECTION("Program will be loaded properly") {
        for(int i = 0; i < 5; i++) {
            REQUIRE(vm2->load_word(i*4) == program[i]);
        }

        for(int i = 5*4; i < 128; i++) {
            REQUIRE(vm2->load_byte(i) == 0);
        }
    }
}

TEST_CASE("Register get & set are functioning properly", "[get_register][set_register]") {
    Emulator* vm = new Emulator(128);

    SECTION("Register 0 is always 0") {
        REQUIRE(vm->get_register(0) == 0);
        vm->set_register(0, 1);
        REQUIRE(vm->get_register(0) == 0);
    }

    SECTION("Registers 1-31 can be set to any value") {
        for(int i = 1; i < 32; i++) {
            vm->set_register(i, 5);
            REQUIRE(vm->get_register(i) == 5);
        }
    }
}

TEST_CASE("Memory operations are functioning properly", "[load_byte][store_byte][load_word][store_word]") {
    Emulator* vm = new Emulator(128);

    SECTION("Can individually read and write bytes on any part of the memory") {
        for(int i = 1; i < 128; i++) {
            vm->store_byte(0xff, i);
            REQUIRE(vm->load_byte(i) == 0xff);
        }
    }

    SECTION("Write words as four bytes") {
        vm->store_word(0xffffffff, 0);
        REQUIRE(vm->load_byte(0) == 0xff);
        REQUIRE(vm->load_byte(1) == 0xff);
        REQUIRE(vm->load_byte(2) == 0xff);
        REQUIRE(vm->load_byte(3) == 0xff);
    }

    SECTION("Load word from four bytes") {
        vm->store_word(0xffffffff, 0);
        REQUIRE(vm->load_word(0) == 0xffffffff);
    }

    SECTION("Write words in Little-Endian") {
        vm->store_word(0x01020304, 0);
        REQUIRE(vm->load_byte(0) == 0x04);
        REQUIRE(vm->load_byte(1) == 0x03);
        REQUIRE(vm->load_byte(2) == 0x02);
        REQUIRE(vm->load_byte(3) == 0x01);
    }

    SECTION("Read words in Little-Endian") {
        vm->store_word(0x01020304, 0);
        REQUIRE(vm->load_word(0) == 0x01020304);
    }
}

TEST_CASE("Test ALU operations", "[step][ALU]") {
    Emulator* vm;

    SECTION("add") {
        WORD program[1];
        program[0] = Utilities::R_instruction(0x00, 3, 1, 2, 0, 0x20); // add r3, r1, r2
        vm = new Emulator(128, program, 1);

        SECTION("functions as expected with positive, normal input") {
            vm->set_register(1, 2);
            vm->set_register(2, 3);

            REQUIRE(vm->step() == 0);
            REQUIRE(vm->get_register(3) == 5);
        }

        SECTION("functions as expected with negative, normal input") {
            vm->set_register(1, -2);
            vm->set_register(2, -3);

            REQUIRE(vm->step() == 0);
            REQUIRE(vm->get_register(3) == -5);
        }

        SECTION("functions as expected with mixed, normal input") {
            vm->set_register(1, -2);
            vm->set_register(2, 3);

            REQUIRE(vm->step() == 0);
            REQUIRE(vm->get_register(3) == 1);
        }

        SECTION("traps on overflow") {
            vm->set_register(1, 0x7FFFFFFF);
            vm->set_register(2, 1);

            REQUIRE(vm->step() == 1);
        }

        SECTION("doesn't trap on overflow bound") {
            vm->set_register(1, 0x7FFFFFFF);
            vm->set_register(2, 0);

            REQUIRE(vm->step() == 0);
        }

        SECTION("traps on underflow") {
            vm->set_register(1, 0x80000000);
            vm->set_register(2, -1);

            REQUIRE(vm->step() == 1);
        }

        SECTION("doesn't trap on underflow bound") {
            vm->set_register(1, 0x80000000);
            vm->set_register(2, 0);

            REQUIRE(vm->step() == 0);
        }
    }

    SECTION("addu") {
        WORD program[1];
        program[0] = Utilities::R_instruction(0x00, 3, 1, 2, 0, 0x21); // add r3, r1, r2
        vm = new Emulator(128, program, 1);

        SECTION("functions as expected with positive, normal input") {
            vm->set_register(1, 2);
            vm->set_register(2, 3);

            REQUIRE(vm->step() == 0);
            REQUIRE(vm->get_register(3) == 5);
        }

        SECTION("functions as expected with negative, normal input") {
            vm->set_register(1, -2);
            vm->set_register(2, -3);

            REQUIRE(vm->step() == 0);
            REQUIRE(vm->get_register(3) == -5);
        }

        SECTION("functions as expected with mixed, normal input") {
            vm->set_register(1, -2);
            vm->set_register(2, 3);

            REQUIRE(vm->step() == 0);
            REQUIRE(vm->get_register(3) == 1);
        }

        SECTION("doesn't trap on overflow") {
            vm->set_register(1, 0x7FFFFFFF);
            vm->set_register(2, 1);

            REQUIRE(vm->step() == 0);
            REQUIRE(vm->get_register(3) == 0x80000000);
        }

        SECTION("doesn't trap on underflow") {
            vm->set_register(1, 0x80000000);
            vm->set_register(2, -1);

            REQUIRE(vm->step() == 0);
            REQUIRE(vm->get_register(3) == 0x7FFFFFFF);
        }
    }
}
