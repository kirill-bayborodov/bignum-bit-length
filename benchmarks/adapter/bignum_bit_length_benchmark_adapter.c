#include "bignum_bit_length_benchmark_adapter.h"
#include "bignum_bit_length.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define FNV_OFFSET UINT64_C(1469598103934665603)
#define FNV_PRIME UINT64_C(1099511628211)

typedef struct bit_length_benchmark_state {
    bignum_t value;
    size_t bit_length;
} bit_length_benchmark_state_t;

/** @brief Compares two optional workload tokens. */
static int equal_text(const char *left, const char *right)
{
    return left != NULL && right != NULL && strcmp(left, right) == 0;
}

/** @brief Advances the deterministic adapter generator. */
static uint64_t next_value(uint64_t *state)
{
    if (*state == 0U) *state = UINT64_C(0x9e3779b97f4a7c15);
    *state ^= *state << 7U;
    *state ^= *state >> 9U;
    *state ^= *state << 8U;
    return *state;
}

/** @brief Tests whether a token belongs to a NULL-terminated vocabulary. */
static int allowed(const char *value, const char *const *list)
{
    if (value == NULL || list == NULL) return 0;
    for (size_t i = 0U; list[i] != NULL; ++i) if (equal_text(value, list[i])) return 1;
    return 0;
}

/** @brief Maps a workload size token to a valid bignum word length. */
static size_t choose_length(const benchmark_workload_t *workload, uint64_t *state)
{
    if (equal_text(workload->size_profile, "one") || equal_text(workload->size_profile, "tiny")) return 1U;
    if (equal_text(workload->size_profile, "quarter") || equal_text(workload->size_profile, "small")) return BIGNUM_CAPACITY / 4U;
    if (equal_text(workload->size_profile, "half") || equal_text(workload->size_profile, "medium")) return BIGNUM_CAPACITY / 2U;
    if (equal_text(workload->size_profile, "near-capacity") || equal_text(workload->size_profile, "large")) return BIGNUM_CAPACITY;
    return 1U + (size_t)(next_value(state) % (BIGNUM_CAPACITY / 2U));
}

/** @brief Fills a normalized deterministic bignum record. */
static void fill_value(bignum_t *value, size_t length, uint64_t *state, int zero)
{
    memset(value, 0, sizeof(*value));
    if (zero) return;
    value->len = length == 0U ? 1U : length;
    for (size_t word = 0U; word < value->len; ++word) value->words[word] = next_value(state);
    if (value->words[value->len - 1U] == 0U) value->words[value->len - 1U] = 1U;
}

/** @brief Initializes one benchmark state from validated workload metadata. */
static benchmark_adapter_status_t initialize(void *opaque, uint64_t index,
                                              const benchmark_workload_t *workload, void *context)
{
    bit_length_benchmark_state_t *state = opaque;
    uint64_t random_state;
    int zero;
    (void)context;
    if (state == NULL || workload == NULL ||
        bignum_bit_length_benchmark_validate_workload(workload) != BIGNUM_BIT_LENGTH_BENCHMARK_STATUS_SUCCESS)
        return BENCHMARK_ADAPTER_STATUS_INPUT_ERROR;
    random_state = workload->seed ^ (index + UINT64_C(0x9e3779b97f4a7c15));
    zero = equal_text(workload->input_kind, "zero") ||
        (equal_text(workload->input_kind, "mixed") && (index & 1U));
    fill_value(&state->value, choose_length(workload, &random_state), &random_state, zero);
    state->bit_length = 0U;
    return BENCHMARK_ADAPTER_STATUS_SUCCESS;
}

/** @brief Executes one bit-length query on the benchmark state. */
static benchmark_adapter_status_t operation(void *opaque, uint64_t iteration,
                                             const benchmark_workload_t *workload, void *context)
{
    bit_length_benchmark_state_t *state = opaque;
    (void)iteration; (void)workload; (void)context;
    if (state == NULL || bignum_bit_length(&state->value, &state->bit_length) != BIGNUM_BIT_LENGTH_SUCCESS)
        return BENCHMARK_ADAPTER_STATUS_OPERATION_ERROR;
    return BENCHMARK_ADAPTER_STATUS_SUCCESS;
}

/** @brief Hashes the complete source record and output into the checksum. */
static uint64_t checksum(const void *opaque, uint64_t iteration, void *context)
{
    const bit_length_benchmark_state_t *state = opaque;
    uint64_t hash = FNV_OFFSET;
    (void)context;
    if (state == NULL) return 0U;
    for (size_t word = 0U; word < BIGNUM_CAPACITY; ++word) {
        hash ^= state->value.words[word];
        hash *= FNV_PRIME;
    }
    hash ^= state->value.len; hash *= FNV_PRIME;
    hash ^= state->bit_length; hash *= FNV_PRIME;
    return hash ^ iteration;
}

bignum_bit_length_benchmark_status_t bignum_bit_length_benchmark_validate_workload(
    const benchmark_workload_t *workload)
{
    static const char *const input[] = { "zero", "nonzero", "mixed", NULL };
    static const char *const operation[] = {
        "bit-length", "bit-zero", "bit-word", "bit-random", "bit-mixed",
        "noop", "default", "mixed", NULL
    };
    static const char *const measure[] = { "end-to-end", "kernel-only", NULL };
    static const char *const size[] = {
        "one", "quarter", "half", "variable", "near-capacity", "tiny", "small", "medium", "large", NULL
    };
    static const char *const capacity[] = { "normal", "near-capacity", NULL };
    if (workload == NULL) return BIGNUM_BIT_LENGTH_BENCHMARK_STATUS_NULL_ARGUMENT;
    if (!allowed(workload->input_kind, input) || !allowed(workload->operation_kind, operation) ||
        !allowed(workload->measure_mode, measure) || !allowed(workload->size_profile, size) ||
        !allowed(workload->capacity_profile, capacity)) return BIGNUM_BIT_LENGTH_BENCHMARK_STATUS_INVALID_PROFILE;
    return BIGNUM_BIT_LENGTH_BENCHMARK_STATUS_SUCCESS;
}

bignum_bit_length_benchmark_status_t bignum_bit_length_benchmark_adapter_init(benchmark_adapter_t *adapter)
{
    if (adapter == NULL) return BIGNUM_BIT_LENGTH_BENCHMARK_STATUS_NULL_ARGUMENT;
    *adapter = (benchmark_adapter_t){
        .benchmark_name = "bignum_bit_length",
        .state_size = sizeof(bit_length_benchmark_state_t),
        .success_code = BENCHMARK_ADAPTER_STATUS_SUCCESS,
        .adapter_context = NULL,
        .initialize = initialize,
        .operation = operation,
        .checksum = checksum
    };
    return BIGNUM_BIT_LENGTH_BENCHMARK_STATUS_SUCCESS;
}
