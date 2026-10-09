/*
 * 大整数乘法（Karatsuba 分治法）详细注释版
 * 输入：两行字符串，表示两个大整数
 * 输出：两数乘积
 *
 * 存储方式：低位在前，高位在后。
 * 例如整数 123，存为 vector<int>{3, 2, 1}
 * 好处是进位从下标 0（个位）开始向高位传递，处理很方便。
 */

#include <bits/stdc++.h>
using namespace std;

// VI 是 vector<int> 的别名，用来表示一个高精度整数（动态数组）
using VI = vector<int>;

// ---------------- 基础工具：trim ----------------
// 去除高位的多余 0。
// 因为是低位在前，所以高位在 vector 的末尾。
// 如果末尾是 0，就弹出去（pop_back），直到只剩一个元素或者末尾不是 0。
// 例如：{0, 0, 0} 表示 0，trim 后保留 {0}。
VI trim(VI a) {
    while (a.size() > 1 && a.back() == 0) a.pop_back();
    if (a.empty()) a.push_back(0);
    return a;
}

// ---------------- 高精度加法：add ----------------
// 模拟手算加法，从低位到高位相加，处理进位。
// carry 是“进位”：上一位相加超过 9 时，传给下一位的数值。
// 结果数组长度取 max(len(a), len(b)) + 1，预留最高位的进位。
VI add(const VI& a, const VI& b) {
    // size_t 是无符号整数类型，用来表示大小、长度、下标。
    // vector 的 size() 返回的就是 size_t。
    VI c(max(a.size(), b.size()) + 1, 0); 
    int carry = 0; // 进位，初始为 0

    for (size_t i = 0; i < c.size(); ++i) {
        // 当前位的和 = 进位 + a的当前位 + b的当前位
        int sum = carry;
        if (i < a.size()) sum += a[i]; // 如果 a 还有这一位，加上
        if (i < b.size()) sum += b[i]; // 如果 b 还有这一位，加上

        c[i] = sum % 10;   // 当前位只保留个位数字
        carry = sum / 10;  // 十位及以上作为新的进位，传给下一轮
    }
    return trim(c); // 去掉最高位可能多出来的 0
}

// ---------------- 高精度减法：sub ----------------
// 模拟手算减法，要求 a >= b。
// borrow 是“借位”：当前位不够减时，向高位借 1，当前位加 10。
VI sub(const VI& a, const VI& b) {
    VI c;             // 结果数组
    int borrow = 0;   // 借位，初始为 0

    for (size_t i = 0; i < a.size(); ++i) {
        // 当前位 = a的当前位 - 借位 - b的当前位（如果 b 有这一位）
        int cur = a[i] - borrow - (i < b.size() ? b[i] : 0);

        if (cur < 0) {
            // 不够减，向高位借 1，当前位加 10
            cur += 10;
            borrow = 1;
        } else {
            borrow = 0; // 够减，不需要借位
        }
        c.push_back(cur);
    }
    return trim(c);
}

// ---------------- 左移：shift ----------------
// 相当于乘以 10^k。
// 在低位在前的数组里，乘以 10^k 等价于在数组前面补 k 个 0。
// 例如 a = {3, 2, 1} 表示 123，shift(a, 2) 得到 {0, 0, 3, 2, 1} 表示 12300。
VI shift(const VI& a, int k) {
    if (a.size() == 1 && a[0] == 0) return {0}; // 0 左移还是 0
    
    VI c(k, 0); // 先创建 k 个 0
    c.insert(c.end(), a.begin(), a.end()); // 把 a 的所有位接在后面
    return trim(c);
}

// ---------------- 普通乘法：naiveMul ----------------
// 用于小规模递归基（Karatsuba 缩小到一定程度时调用）。
// 核心思想：双重循环模拟竖式乘法，先累加，最后统一处理进位。
VI naiveMul(const VI& a, const VI& b) {
    // 特判 0
    if ((a.size() == 1 && a[0] == 0) || (b.size() == 1 && b[0] == 0))
        return {0};

    // 结果数组长度最多为 a.size() + b.size()
    VI c(a.size() + b.size(), 0);

    // 双重循环：a 的每一位乘以 b 的每一位
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] == 0) continue; // 当前位是 0，跳过优化
        for (size_t j = 0; j < b.size(); ++j) {
            // 核心公式：c[i+j] 累加 a[i] * b[j]
            // 因为 a[i] 是第 i 位，b[j] 是第 j 位，乘积应放在第 i+j 位。
            // 这里先累加，不处理进位，避免进位影响后面很多位。
            c[i + j] += a[i] * b[j];
        }
    }

    // 统一处理进位（从低位到高位）
    int carry = 0;
    for (size_t i = 0; i < c.size(); ++i) {
        int val = c[i] + carry;
        c[i] = val % 10;   // 保留个位
        carry = val / 10;  // 进位传给下一位
    }

    return trim(c);
}

// ---------------- 核心算法：Karatsuba 分治乘法 ----------------
// 原理：将大整数拆成高低位，用 3 次乘法代替普通分治的 4 次乘法。
// 设 a = a1*10^m + a0, b = b1*10^m + b0
// 则 a*b = z0 + z1*10^m + z2*10^(2m)
// 其中：
//   z0 = a0 * b0
//   z2 = a1 * b1
//   z1 = (a0+a1)*(b0+b1) - z0 - z2
VI karatsuba(const VI& a, const VI& b) {
    // 空数组或 0 的情况
    if (a.empty() || b.empty()) return {0};
    if ((a.size() == 1 && a[0] == 0) || (b.size() == 1 && b[0] == 0))
        return {0};

    // 递归基：长度较小（<= 32）时直接用普通乘法，避免递归开销
    if (min(a.size(), b.size()) <= 32) {
        return naiveMul(a, b);
    }

    // n 取两个数中较大的长度，m 是分割点
    size_t n = max(a.size(), b.size());
    size_t m = n / 2;

    // 准备拆分后的高低位
    VI a0, a1, b0, b1;

    // 拆分 a = a0 + a1 * 10^m
    // a0 是低 m 位（前 m 个元素），a1 是高位（剩余元素）
    if (a.size() > m) {
        a0.assign(a.begin(), a.begin() + m);
        a1.assign(a.begin() + m, a.end());
    } else {
        a0 = a;
        a1 = {0};
    }

    // 拆分 b = b0 + b1 * 10^m
    if (b.size() > m) {
        b0.assign(b.begin(), b.begin() + m);
        b1.assign(b.begin() + m, b.end());
    } else {
        b0 = b;
        b1 = {0};
    }

    // 递归计算三个关键乘积
    VI z0 = karatsuba(a0, b0);   // z0 = a0 * b0
    VI z2 = karatsuba(a1, b1);   // z2 = a1 * b1

    // 计算 (a0 + a1) 和 (b0 + b1)
    VI a01 = add(a0, a1);
    VI b01 = add(b0, b1);

    // z = (a0+a1)*(b0+b1)
    VI z1 = karatsuba(a01, b01);
    
    // 核心公式：z1 = z - z0 - z2
    z1 = sub(z1, add(z0, z2));

    // 合并结果：
    // result = z0 + z1 * 10^m + z2 * 10^(2m)
    VI res = add(add(z0, shift(z1, m)), shift(z2, 2 * m));

    return trim(res);
}

// ---------------- 主函数 ----------------
int main() {
    // 关闭同步，加速输入输出
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string s1, s2;
    cin >> s1 >> s2; // 读入两个大整数字符串

    // 如果是 0，直接输出
    if (s1 == "0" || s2 == "0") {
        cout << 0 << '\n';
        return 0;
    }

    // 将字符串转换为低位在前的 vector<int>
    VI a, b;
    // 从字符串末尾（个位）开始，依次存入 vector
    for (int i = (int)s1.size() - 1; i >= 0; --i)
        a.push_back(s1[i] - '0');
    for (int i = (int)s2.size() - 1; i >= 0; --i)
        b.push_back(s2[i] - '0');

    // 调用 Karatsuba 算法
    VI ans = karatsuba(a, b);

    // 从高位到低位输出结果（vector 末尾是高位）
    for (int i = (int)ans.size() - 1; i >= 0; --i)
        cout << ans[i];
    cout << '\n';

    return 0;
}