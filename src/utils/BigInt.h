#pragma once

#include <openssl/bn.h>

namespace gtb
{

class BigInt
{
    public:
        BigInt(uint64_t val = 0);
        BigInt(BigInt &&);
        BigInt(const BigInt &);
        BigInt &operator=(BigInt &&);
        BigInt &operator=(const BigInt &);
        ~BigInt();

        operator bool() const;
        uint64_t toUint64() const;
        int64_t toInt64() const;
        bool isNegative() const;

        // arithmetic assignment
        BigInt &operator+=(const BigInt &rhs);
        BigInt &operator-=(const BigInt &rhs);
        BigInt &operator*=(const BigInt &rhs);
        BigInt &operator/=(const BigInt &rhs);

        // binary operators
        friend BigInt operator+(BigInt lhs, const BigInt &rhs);
        friend BigInt operator-(BigInt lhs, const BigInt &rhs);
        friend BigInt operator*(BigInt lhs, const BigInt &rhs);
        friend BigInt operator/(BigInt lhs, const BigInt &rhs);

        // comparisons
        friend bool operator==(const BigInt &lhs, const BigInt &rhs);
        friend bool operator!=(const BigInt &lhs, const BigInt &rhs);
        friend bool operator<(const BigInt &lhs, const BigInt &rhs);
        friend bool operator>(const BigInt &lhs, const BigInt &rhs);
        friend bool operator<=(const BigInt &lhs, const BigInt &rhs);
        friend bool operator>=(const BigInt &lhs, const BigInt &rhs);

    private:
        BIGNUM *bn;
};

}
