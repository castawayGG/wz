// main entry point for unit tests
#include <cstdio>

int TestAxisProcessorMain();
int TestMappingEngineMain();
int TestMacroEngineMain();
int TestProfileManagerMain();

int main() {
    int failures = 0;
    failures += TestAxisProcessorMain();
    failures += TestMappingEngineMain();
    failures += TestMacroEngineMain();
    failures += TestProfileManagerMain();

    printf("=== TOTAL: %s (%d failures) ===\n",
        failures == 0 ? "ALL PASSED" : "SOME FAILED", failures);
    return failures ? 1 : 0;
}
