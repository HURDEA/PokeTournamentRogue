#pragma once

class Tests {
public:
    static void runAllTests();
private:
    static void testDomainAndRepository();
    static void testControllerAndMemoryTimeline();
    static void testAdvancedFiltering();
};