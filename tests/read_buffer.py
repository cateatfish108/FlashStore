import struct
import pandas as pd

df = pd.DataFrame(columns=['last_price', 'open_price'])
data_list = []
# 打开二进制文件
with open("mmap/shm.flash_store.1.journal", "rb") as f:
    hex_data = " ".join(f"{byte:02x}" for byte in f.read(16))
    print(hex_data)
    f.seek(137,0)
    # 循环读取并解析数据
    cnt = 0
    continue_cnt = 0
    while True:
        # 偏移指定字节数
        frame_header_buf = f.read(20)  # 从当前位置向后偏移 2 个字节
        frame_header = struct.unpack('=bqiIhb', frame_header_buf)
        frame_size = frame_header[2]
        if frame_size < 20:
            print(f"解析结束,frame_size:{frame_size} total:",cnt)
            break
        # 读取数据
        # data = f.read(8)  # 读取 4 个字节的数据
        data = f.read(40)  # 读取 4 个字节的数据
        
        # 判断是否读到空数据（全零）
        # if all(byte == 0 for byte in data):
        #     print("解析结束,total:",cnt)
        #     continue_cnt+=1
        #     if continue_cnt>=100:
        #         break
            # continue
        # hex_data = " ".join(f"{byte:02x}" for byte in data)
        # print(hex_data)
        # print(data)
        # 判断是否已经读到文件末尾
        if not data:
            break
        
        # 解析数据
        # parsed_data = struct.unpack('ii', data)
        parsed_data = struct.unpack('ddddq', data)
        # 打印解析结果
        # print("解析结果:", parsed_data)
        # df = df.append({'last_price': parsed_data[0], 'open_price': parsed_data[1]}, ignore_index=True)
        # data_list.append({'last_price': parsed_data[0], 'open_price': parsed_data[1]})
        data_list.append({'last_price': parsed_data[0], 'open_price': parsed_data[1],'highest_price': parsed_data[2], 'lowest_price': parsed_data[3],'local_timestamp_us': parsed_data[4]})
        
        # df.to_csv("quote_data.csv",index=False)
        cnt+=1
        # break
# 将列表转换为 DataFrame
df = pd.DataFrame(data_list)
# 打印 DataFrame
df.to_csv("quote_data.csv",index=False)
# print(df)
        