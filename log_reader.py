import matplotlib.pyplot as plt
import numpy as np

log = open("log.txt", "r")

log_san = log.read().split("Ejecutando ")[1:] # [mergesorts, quicksorts]
log_mergesort = log_san[0]
log_mergesort = log_mergesort.split("...")[1]
log_quicksort = log_san[1]
log_quicksort = log_quicksort.split("...")[1]

def time_diff(time1, time2): #formato año-mes-dia hora:minuto:segundo
    """
    Returns the difference between two time strings in seconds.
    """
    from datetime import datetime
    time1 = datetime.strptime(time1, "%Y-%m-%d %H:%M:%S")
    time2 = datetime.strptime(time2, "%Y-%m-%d %H:%M:%S")
    return (time2 - time1).total_seconds()
    

def read_log(log_list):
    """
    Reads the log file and returns a list of dicts with the following keys:
    (sortType, Xi, M, B, IOs, Tiempo)
    """
    return_list = []
    log_list_san = log_list.split("No se pudo eliminar el archivo (puede que no exista): arreglo.bin\n")[1:]
    for data in log_list_san:
        data = data.split("\n")
        data = data[:2]
        init_time = str(data[0].split("=")[5]).split(",")[0]
        data = data[1].split(",")
        # for i in range(len(data)):
        data_i = data.__str__().split("=")
        # print(data_i)
        new_data = {
            "sortType" : data_i[0].split(":")[0][2:],
            "Xi" : data_i[1].split("', ")[0],
            "M" : data_i[2].split("', ")[0],
            "B" : data_i[3].split("', ")[0],
            "IOs" : data_i[4].split("', ")[0],
            "Tiempo" : time_diff(init_time, data_i[5].split(" ")[0] + ' ' + data_i[5].split(" ")[1].split("'")[0])
            }
        return_list.append(new_data)
    return return_list
               
            

log_merge_data = read_log(log_mergesort)
for i in range(len(log_merge_data)):
    print(log_merge_data[i])

log_quick_data = read_log(log_quicksort)
for i in range(len(log_quick_data)):
    print(log_quick_data[i])

# graficar los resultados
def plot_data(log_data, sort_type):
    """
    Plots the data from the log file.
    """
    x = []
    y = []
    for data in log_data:
        x.append(int(data["Xi"]))
        y.append(data["Tiempo"])
    
    plt.plot(x, y, label=sort_type)
    plt.xlabel("Tamaño del arreglo")
    plt.ylabel("Tiempo (s)")
    plt.title("Tiempo de ejecución de " + sort_type)
    plt.legend()
    plt.show()
plot_data(log_merge_data, "Mergesort")
plot_data(log_quick_data, "Quicksort")