import requests

#  request(
#         self,
#         method,
#         url,
#         params=None,
#         data=None,
#         headers=None,
#         cookies=None,
#         files=None,
#         auth=None,
#         timeout=None,
#         allow_redirects=True,
#         proxies=None,
#         hooks=None,
#         stream=None,
#         verify=None,
#         cert=None,
#         json=None,
#     )


# r = requests.get("https://www.nokia.com/bell-labs/about/dennis-m-ritchie/btut.pdf")

# print(f"Status: {r.status_code}")
# print("Headers:")
# for k, v in r.headers.items():
#     print(f"{k}: {v}")
# print()
# print(r.text)

import requests

r = requests.get("https://www.nokia.com/bell-labs/about/dennis-m-ritchie/btut.pdf")

print(f"Status: {r.status_code}")
print("--------------------------------")

print("Headers:")
print("--------------------------------")

for k, v in r.headers.items():
    print(f"{k}: {v}")
print("--------------------------------")
print(r.text)

