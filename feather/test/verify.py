import pandas as pd

# 两维索引验证
info = {
    'name' : {
        'name_1' : 'zarten_1',
        'name_2' : 'zarten_2'
    },
    'age' : {
        'age_1' : 18,
        'age_2' : 19
    }
}

index_two = pd.DataFrame(info)
print(index_two)
index_two.to_csv("/home/slshen/feather/data/index_two.csv")
index_new=index_two.reset_index(drop=False)
print(index_new)
index_new.to_feather("/home/slshen/feather/data/index_two.feather")


# 嵌套列表验证
nest_list = [
    {
        "Fruit": [  {"Price": 15.2, "Quality": "A"},
                    {"Price": 19, "Quality": "B"},
                    {"Price": 17.8, "Quality": "C"},],
        "Name": "Orange"
    },

    {
        "Fruit": [{"Price": 23.2, "Quality": "A"},
                {"Price": 28, "Quality": "B"}],
        "Name": "Grapes"
    }
]
nest = pd.DataFrame(nest_list)
print(nest)

nest.to_csv("/home/slshen/feather/data/nest_list.csv")
nest.to_feather("/home/slshen/feather/data/nest_list.feather")