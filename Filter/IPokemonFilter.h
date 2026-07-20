#pragma once
#include "../Domain/Pokemon.h"
#include <string>

// The Base Specification Interface
class IPokemonFilter {
public:
    virtual bool satisfies(const Pokemon& p) const = 0;
    virtual ~IPokemonFilter() = default;
};

// Concrete Strategy: Filter by Type
class TypeFilter : public IPokemonFilter {
private:
    std::string targetType;
public:
    explicit TypeFilter(const std::string& type) : targetType(type) {}
    bool satisfies(const Pokemon& p) const override {
        if (targetType == "None" || targetType == "All Types") return true;
        return p.getType1() == targetType || p.getType2() == targetType;
    }
};

// Composite Strategy: AND Logic
class AndFilter : public IPokemonFilter {
private:
    const IPokemonFilter& f1;
    const IPokemonFilter& f2;
public:
    AndFilter(const IPokemonFilter& filter1, const IPokemonFilter& filter2) : f1(filter1), f2(filter2) {}
    bool satisfies(const Pokemon& p) const override {
        return f1.satisfies(p) && f2.satisfies(p);
    }
};

// Composite Strategy: OR Logic
class OrFilter : public IPokemonFilter {
private:
    const IPokemonFilter& f1;
    const IPokemonFilter& f2;
public:
    OrFilter(const IPokemonFilter& filter1, const IPokemonFilter& filter2) : f1(filter1), f2(filter2) {}
    bool satisfies(const Pokemon& p) const override {
        return f1.satisfies(p) || f2.satisfies(p);
    }
};