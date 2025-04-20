import random
import ShareMemory

jw = ShareMemory.create_writer("./mmap", "flash_store", "writer")
print(dir(ShareMemory))

tick = ShareMemory.TestD()
tick.last_price = 12345
tick.open_price = 54321
jw.write_data_TestD(tick,0,0)
# tick.open_price123 = 999123
# print(dir(tick))
# print(tick.last_price)
# while True:
for i in range(100000):
    tick.last_price = random.randint(0, 100)
    tick.open_price = random.randint(0, 100)
    jw.write_data_TestD(tick,0,0)
tick.last_price = 66699
tick.open_price = 999
jw.write_data_TestD(tick,0,0)
# print(jw.init("123","456"))
