from dotenv import dotenv_values

Import("env")

config = dotenv_values(".env")

for key, value in config.items():
    env.Append(
        CPPDEFINES=[
            (f"ENV_{key}", '\\"{}\\"'.format(value))
        ]
    )