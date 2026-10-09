#include <bits/stdc++.h>
using namespace std;

// 使用 VI 表示一个高精度整数
// 存储方式：低位在前，高位在后
// 例如整数 123 存储为 vector<int>{3, 2, 1}
using VI = vector<int>;

// 去除高位的多余 0（低位在前，所以是去除 vector 末尾的 0）
VI trim(VI a) {
    while (a.size() > 1 && a.back() == 0) a.pop_back();
    if (a.empty()) a.push_back(0);
    return a;
}

// 高精度加法：计算 a + b
VI add(const VI& a, const VI& b) {
    VI c(max(a.size(), b.size()) + 1, 0);
    int carry = 0;
    for (size_t i = 0; i < c.size(); ++i) {
        int sum = carry;
        if (i < a.size()) sum += a[i];
        if (i < b.size()) sum += b[i];
        c[i] = sum % 10;
        carry = sum / 10;
    }
    return trim(c);
}

// 高精度减法：计算 a - b，要求 a >= b
VI sub(const VI& a, const VI& b) {
    VI c;
    int borrow = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        int cur = a[i] - borrow - (i < b.size() ? b[i] : 0);
        if (cur < 0) {
            cur += 10;
            borrow = 1;
        } else {
            borrow = 0;
        }
        c.push_back(cur);
    }
    return trim(c);
}

// 左移 k 位，相当于乘以 10^k
VI shift(const VI& a, int k) {
    if (a.size() == 1 && a[0] == 0) return {0};
    VI c(k, 0);
    c.insert(c.end(), a.begin(), a.end());
    return trim(c);
}

// 普通 O(n^2) 乘法，用于递归到较小规模时
VI naiveMul(const VI& a, const VI& b) {
    if ((a.size() == 1 && a[0] == 0) || (b.size() == 1 && b[0] == 0))
        return {0};

    VI c(a.size() + b.size(), 0);

    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] == 0) continue;
        for (size_t j = 0; j < b.size(); ++j) {
            c[i + j] += a[i] * b[j];
        }
    }

    int carry = 0;
    for (size_t i = 0; i < c.size(); ++i) {
        int val = c[i] + carry;
        c[i] = val % 10;
        carry = val / 10;
    }

    return trim(c);
}

// Karatsuba 分治乘法
VI karatsuba(const VI& a, const VI& b) {
    if (a.empty() || b.empty()) return {0};
    if ((a.size() == 1 && a[0] == 0) || (b.size() == 1 && b[0] == 0))
        return {0};

    if (min(a.size(), b.size()) <= 32) {
        return naiveMul(a, b);
    }

    size_t n = max(a.size(), b.size());
    size_t m = n / 2;

    VI a0, a1, b0, b1;

    if (a.size() > m) {
        a0.assign(a.begin(), a.begin() + m);
        a1.assign(a.begin() + m, a.end());
    } else {
        a0 = a;
        a1 = {0};
    }

    if (b.size() > m) {
        b0.assign(b.begin(), b.begin() + m);
        b1.assign(b.begin() + m, b.end());
    } else {
        b0 = b;
        b1 = {0};
    }

    VI z0 = karatsuba(a0, b0);
    VI z2 = karatsuba(a1, b1);

    VI a01 = add(a0, a1);
    VI b01 = add(b0, b1);

    VI z1 = karatsuba(a01, b01);
    z1 = sub(z1, add(z0, z2));

    VI res = add(add(z0, shift(z1, m)), shift(z2, 2 * m));
    return trim(res);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string s1, s2;
    cin >> s1 >> s2;

    if (s1 == "0" || s2 == "0") {
        cout << 0 << '\n';
        return 0;
    }

    VI a, b;
    for (int i = (int)s1.size() - 1; i >= 0; --i)
        a.push_back(s1[i] - '0');
    for (int i = (int)s2.size() - 1; i >= 0; --i)
        b.push_back(s2[i] - '0');

    VI ans = karatsuba(a, b);

    for (int i = (int)ans.size() - 1; i >= 0; --i)
        cout << ans[i];
    cout << '\n';

    return 0;
}