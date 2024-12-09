import random

insert_template = "INSERT INTO table VALUES ({}, {}, {})\n"
select_template = "SELECT attr2, attr3 FROM table WHERE attr1={} AND attr3={}\n"
update_template = "UPDATE table SET attr1={}, attr2={} WHERE attr1={}\n"
delete_template = "DELETE FROM table WHERE attr1={}\n"

# Field values for randomization
attr1_values = ["Comedy", "SciFi", "Drama", "Fantasy", "Romance", "Horror", "Action", "Thriller", "Crime", "History", "Musical", "Mystery"]
attr2_values = ["Miramax", "Paramount", "Disney", "WarnerBros", "Universal", "Fox", "Lionsgate", "DreamWorks", "Cinemax", "Columbia"]
year_range = range(1900, 2025)
sizes = [100, 200, 500, 1000, 2000, 5000, 10000, 20000, 50000, 100000]

def generate_biased_sql(n, insert_bias=70):
    assert 0 <= insert_bias <= 100, "Insert bias must be between 0 and 100."
    
    # Probabilities for operations
    select_bias = (100 - insert_bias) // 3
    update_bias = select_bias
    delete_bias = 100 - insert_bias - select_bias - update_bias
    
    operations = (
        ["INSERT"] * insert_bias +
        ["SELECT"] * select_bias +
        ["UPDATE"] * update_bias +
        ["DELETE"] * delete_bias
    )
    
    sql_statements = []
    for _ in range(n):
        op = random.choice(operations)
        if op == "INSERT":
            sql_statements.append(
                insert_template.format(
                    random.choice(attr1_values),
                    random.choice(attr2_values),
                    random.choice(year_range)
                )
            )
        elif op == "SELECT":
            sql_statements.append(
                select_template.format(
                    random.choice(attr1_values),
                    random.choice(year_range)
                )
            )
        elif op == "UPDATE":
            sql_statements.append(
                update_template.format(
                    random.choice(attr1_values),
                    random.choice(attr2_values),
                    random.choice(attr1_values)
                )
            )
        elif op == "DELETE":
            sql_statements.append(
                delete_template.format(random.choice(attr1_values))
            )
    return "".join(sql_statements)

biased_file_paths = []
insert_bias = 70
for size in sizes:
    sql_content = generate_biased_sql(size, insert_bias=insert_bias)
    file_path = f"sql_{size}_insertBias70.sql"
    with open(file_path, "w") as file:
        file.write(sql_content)
    biased_file_paths.append(file_path)

biased_file_paths
