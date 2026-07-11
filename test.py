import desbordante

TABLE = ("test_input_data/Test1.csv", ",", True)
DOMAIN = desbordante.pac.domains.Ball(["3"], 3)

algo = desbordante.pac_verification.algorithms.DomainPACVerifier()

algo.load_data(table=TABLE, domain=DOMAIN, column_indices=[0])

if 0:
    # Works fine with this line
    algo.set_option("min_epsilon", 1)
algo.set_option("max_epsilon", 3)
