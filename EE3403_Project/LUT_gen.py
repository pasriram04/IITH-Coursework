import numpy as np

n_y_bits = 7
n_r_bits = 8+1
n_m_bits = 28

y_values = np.linspace(1, 2, 2**(n_y_bits - 1), endpoint=False)

r_values = 1 / y_values
r_fixed = np.floor(r_values * (2**(n_r_bits - 1))).astype(int)

m_values = 1 / np.sqrt(r_fixed / (2**(n_r_bits - 1)))
m_fixed = np.floor(m_values * (2**(n_m_bits - 1))).astype(int)

x = m_fixed[32]
print(f"{x:0{n_m_bits}b}\n")

with open("lutR.mem", "w") as r_file:
    for r in r_fixed:
        r_file.write(f"{r:0{n_r_bits}b}\n")

with open("lutM.mem", "w") as m_file:
    for i, m in enumerate(m_fixed):
        binary_representation = f"{m:0{n_m_bits}b}"
        print(f"Index {i}: Decimal {m/(2**27)}, Binary {binary_representation}")
        m_file.write(binary_representation + "\n")

print("LUT files generated: Rlut.mem and Mlut.mem")
