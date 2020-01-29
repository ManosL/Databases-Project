import random
import time

def main():
    names = ["Ned","Walter","Skyler","Hank","Jax","Clay","Frank","Claire",
    "Catelyn","Jon","Robb","Sansa","Arya","Roose","Ramsay","Hannah","Tony",
    "Dolores","Bobby","Jemma","Ragnar","Ivar","Bjorn","Alexander","Conor"]

    surnames = ["Stark","Bolton","Lannister","Baratheon","Frey","Lothbrok",
    "Teller","McGregor","Gustafsson","Cyborg","Baker","Arryn","House","Jones",
    "Tyson","Mayweather","Tyrell","Targaryen","Eubanks","Shevchenko","Jordan",
    "Pettis","Ferguson","Lewis","Johnson"]

    addresses = ["Athens","Ioannina","Thesaloniki","Patra","Kalamata","Rhodes",
    "Volos","Chania","Irakleio","Rethymno","Lasithi","Washington","Kos",
    "Mexico","Brasil","Toronto","Calgary","Rome","Fiorentina","Sicilly","Canberra","Sydney",
    "Madrid","Lisbon","Cardiff"]

    seen_nums = set()

    random.seed(time.time())

    reg_file = open("records.txt","w")

    final_strings = []

    for name in names:
        for surname in surnames:
            for address in addresses:
                id = random.randint(1,3 * (10**4))
                while id in seen_nums:
                    if id <= 30000:
                        id += 1
                    else:
                        id = 1
                else:
                    seen_nums.add(id)
                    
                final_strings.append('{' + str(id) + ',"' + name + '","' + surname + '","' + address + '"}\n')
    
    reg_file.writelines(final_strings)

    reg_file.close()

if __name__ == "__main__":
    main()