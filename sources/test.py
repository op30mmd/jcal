import pyjalali.jtime
import time

# Test a date after 2038
t = 2**32
print(pyjalali.jtime.jctime(t))
