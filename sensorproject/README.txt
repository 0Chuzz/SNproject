PROGETTO SENSOR NETWORKS

datasets:
http://4drepository.inrialpes.fr/public/viewgroup/6
http://www.iipl.fudan.edu.cn/~zhangjp/Dataset/fd_pede_dataset_intro.htm

comando di esempio:
./sensorproject ~/Downloads/snproj/groundtruth/groundtruth/4/foreground_mask/FudanGH10_02685.png


descrizione files:
imageiter.c: ignora avevo fatto un iteratore per muovermi sull'immagine ma è piu semplice quello che ho messo su utils.c
labirithlabeler.c: ignora altra versione dell'algoritmo di labeling su cui sto lavorando
main.c: funzione main, dato che faremo cose diverse probabilmente lavorando avremo due main diversi ma amen
pbmdecoder.c : legge i file pbm (dataset 1) 
pngdecoder.c : legge i file png (dataset 2)
questi due li ho testati solo con i dataset sopra, probabilmente esplodono appena c'è un'immagine con un formato un po' diverso, in caso mandami un'immagine con cui non funzionano
ppmencoder.c : salva l'immagine etichettata su file .ppm
reflabeler.c : implementazione di reference del mio algoritmo, dovresti non averne bisogno
sievelabeler.c : implementazione principale dell'algo di labeling, dovrebbe principalmente interessarti questo file
ci sono due funzioni importanti: sieve_extract estrae un singolo componente su un solo buffer, sieveLabel() crea il buffer direttamente per ppmencoder
utils.c : utilità varie per lavorare sui buffers.


quando lavoriamo su cose diverse è meglio fare due branch e mergiare alla fine ma vedremo