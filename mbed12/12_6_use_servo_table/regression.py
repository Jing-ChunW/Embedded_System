import matplotlib.pyplot as plt
import numpy as np

Ts = 30;   # signal interval
end = 150; # signal end point
n = int(end/Ts)+1;

x = np.linspace(0, end, num=n) # signal vector

# TODO: revise this array to your results
y = np.array([0.000, 5.660, 9.646, 10.842, 11.400, 11.560]) # speed vector
y1 = np.array([0.080, 5.104, 10.207, 13.397, 10.208, 11.483])

z = np.polyfit(x, y, 2) # Least squares polynomial fit, and return the coefficients.
z1 = np.polyfit(x, y1, 2)

goal1 = 8
goal = 5             # if we want to let the servo run at 7 cm/sec
                     # equation : z[0]*x^2 + z[1]*x + z[2] = goal
z[2] -= goal         # z[0]*x^2 + z[1]*x + z[2] - goal = 0
z1[2] -=goal1

result = np.roots(z) # Return the roots of a polynomial with coefficients given
result1 = np.roots(z1) # Return the roots of a polynomial with coefficients given
# output the correct one
if (0 <= result[0]) and (result[0] <= end):
    print(result[0])
else:
    print(result[1])
if (0 <= result1[0]) and (result1[0] <= end):
    print(result1[0])
else:
    print(result1[1])