#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

static int write_answer(char *out, size_t cap,
                        const uint32_t *values, size_t count) {
    size_t used = 0;
    if (cap == 0 || count == 0) return 1;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32,
                            i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t low = (uint32_t)challenge;
    uint32_t high = (uint32_t)(challenge >> 32);
    uint32_t middle = (uint32_t)(challenge >> 16);
    uint32_t first = (low + high * 8u) ^ 0x164fu;
    uint32_t second = (high - middle) * 5u + 0x6c24u;
    uint32_t answer = (first * 3u + second) ^ 0xcad1u;
    return write_answer(out, cap, &answer, 1);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[6];
    values[0] = 3u * (uint32_t)((challenge >> 8) & 31u);
    for (unsigned i = 1; i < 6; ++i) {
        uint32_t part = (uint32_t)((challenge >> (21u + 4u * i)) & 63u);
        values[i] = (values[i - 1] + part) ^ (14u * i);
    }
    return write_answer(out, cap, values, 6);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t index = (uint32_t)challenge & 7u;
    uint32_t x = (uint32_t)(challenge >> 3) ^ (uint32_t)(challenge >> 32);
    uint32_t result;
    switch (index) {
        case 0: result = 3u * x + 0xdcecu; break;
        case 1: result = (4u * x) ^ 0x6cd9u; break;
        case 2: result = 5u * x + 0x44c19u; break;
        case 3: result = (x ^ 0xd802u) + (x >> 2); break;
        case 4: result = 5u * x + 0x2328u; break;
        case 5: result = 5u * x + 0x552fu; break;
        case 6: result = (x ^ 0xbc32u) + (x >> 2); break;
        default: result = (4u * x) ^ 0x773bu; break;
    }
    uint32_t values[2] = {index, result};
    return write_answer(out, cap, values, 2);
}

static uint32_t walk(uint32_t target, uint32_t low, uint32_t high,
                     uint32_t salt) {
    uint32_t middle = low + (high - low) / 2u;
    if (target == middle) return (middle + 0x8bu) ^ salt;
    if (target < middle) {
        return (3u * walk(target, low, middle - 1u, salt)) ^
               (middle + salt) ^ 8u;
    }
    return (3u * walk(target, middle + 1u, high, salt)) ^
           (middle + salt) ^ 0x18u;
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t salt = (uint32_t)(challenge >> 7) ^ (uint32_t)(challenge >> 32);
    uint32_t answer = walk((uint32_t)challenge & 127u, 0, 127, salt);
    return write_answer(out, cap, &answer, 1);
}

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    static const unsigned heads[8] = {6, 4, 0, 8, 7, 9, 1, 10};
    static const unsigned next[12] = {8, 10, 3, 5, 0, 11, 4, 9, 7, 1, 2, 12};
    static const uint32_t ids[12] = {
        0x25e, 0x42b, 0x107, 0x763, 0xb1e, 0x822,
        0x537, 0x64f, 0x30b, 0x005, 0x920, 0xa40
    };
    static const uint32_t data[12] = {
        0x9d7b, 0x4903, 0xabaf, 0xb37c, 0x6aac, 0x3f32,
        0xb182, 0x233e, 0xae2a, 0x067c, 0x42ac, 0x83d2
    };
    unsigned node = heads[challenge & 7u];
    unsigned count = (unsigned)((challenge >> 3) & 3u) + 1u;
    uint32_t value = (uint32_t)(challenge >> 6) ^
                     (uint32_t)(challenge >> 32) ^ 0x33b6u;
    uint32_t last = 0;
    for (unsigned i = 0; i < count; ++i) {
        value = (value * 3u + data[node]) ^ ids[node];
        last = ids[node];
        node = next[node];
    }
    uint32_t values[2] = {value, last};
    return write_answer(out, cap, values, 2);
}

static uint32_t operation(unsigned index, uint32_t x) {
    switch (index) {
        case 0: return (x ^ 0x3c38u) + 8u * x;
        case 1:
        case 5: return (x >> 5) | (x << 27);
        case 2:
        case 3: return ((x << 3) | (x >> 29)) ^ 0xcc2fu;
        case 4: return (x + 0xc241u) ^ (x >> 3);
        case 6: return (x ^ (x >> 7)) + 0x247du;
        default: return (2u * x + 0xc6c5u) ^ (7u * x);
    }
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t x = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32);
    uint32_t first = operation((unsigned)(challenge & 7u), x) ^ 0xde95u;
    uint32_t answer = operation((unsigned)((challenge >> 3) & 7u), first) + 0xd39u;
    return write_answer(out, cap, &answer, 1);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    uint32_t x = (uint32_t)challenge ^ (uint32_t)(challenge >> 32);
    uint32_t y = (uint32_t)context ^ (uint32_t)(context >> 32);
    uint32_t route = (uint32_t)context & 3u;
    uint32_t mixed = x ^ y;
    uint32_t second = ((mixed ^ 0x2490u) + ((mixed >> 3) ^ 0x34bdu)) ^
                      (route * 0x2490u) ^ (uint32_t)(context >> 16);
    uint32_t third = second ^ (y + route + 0x34bdu + (x >> 3));
    int size = snprintf(out, cap, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAOPEN%" PRIu32
                        " %" PRIu32 " %" PRIu32, route, second, third);
    return size < 0 || (size_t)size >= cap;
}
