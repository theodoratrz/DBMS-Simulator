## Υλοποίηση Συστημάτων Βάσεων Δεδομένων
### Παύλος Σπανουδάκης
### Θεοδώρα Τροιζή
***
### Δομή & Αρχεία:
- BF64.a (όπως είχε δοθεί)
- Φάκελος record_examples όπως δόθηκε στο eclass
- HP.c , HP.h (Υλοποιήσεις & Υπογραφές HP συναρτήσεων)
- HP_main.c (Main για επίδειξη της λειτουργίας HP)
- HΤ.c , HΤ.h (Υλοποιήσεις & Υπογραφές HΤ συναρτήσεων)
- SHΤ.c , SHΤ.h (Υλοποιήσεις & Υπογραφές HΤ συναρτήσεων)
- SHT_main.c (Main για επίδειξη των HT και SHT)
- Makefile
### Οδηγίες χρήσης:
Τρέχοντας `make` γίνονται όλα τα απαραίτητα compilations (και για τις δύο `main`). Μπορεί να εκτελεστεί οποιαδήποτε από τις δύο σε αυτό το σημείο. Συνίσταται να γίνει redirect του output για ευκολότερη ανάγνωση. Η `HP_main` και η `SHΤ_main`, αν εκτελεστούν, δημιουργούν αρχεία `HP_file` και `HΤ_file`, `SHT_file` αντίστοιχα. Τρέχοντας `make clean` διαγράφονται τα εκτελέσιμα και τα αντικειμενικά αρχεία (όχι όμως και τα αρχεία επιπέδου block που αναφέρθηκαν)
### HP συναρτήσεις:
Έχουν υλοποιηθεί τα ζητόυμενα με την λειτουργικότητα που ζητήθηκε στην εκφώνηση.  

Ένα HP αρχείο αποθηκεύει όλες τις πληροφορίες στο πρώτο του μπλοκ, το οποίο δεν περιέχει εγγραφές. Επίσης, στην αρχή του μπλοκ αυτού αποθηκεύεται μια συμβολοσειρά `"heap"` που επιτρέπει να αναγνωρίσουμε αν πρόκειται για heap file ή όχι.

Για την αποθήκευση των Records ακολουθείται Packed σχήμα:
- Στο τέλος του μπλοκ, διατηρείται ένας μετρητής εγγραφών και ένας δείκτης για το επόμενο μπλοκ (-1 αν δεν υπάρχει)
- Η εισαγωγή νέας εγγραφής γίνεται αμέσως μετά την τελευταία υπάρχουσα εγγραφή. Ελέγχεται πρώτα όλο το αρχείο, για να βεβαιωθούμε ότι δεν υπάρχει ήδη εγγραφή με το key που έχει δοθεί.
- Για την διαγραφή μιας εγγραφής, αυτή αντικαθίσταται από την τελευταία και ο μετρητής μειώνεται κατά 1. Αν η εγγραφή ήταν η τελευταία, απλά μειώνεται ο μετρητής.

Κατά την δημιουργία του αρχείου, θα πρέπει το πεδίο `attrName` της `HP_CreateFile` να είναι `"id"`
Ωστόσο, οι συναρτήσεις εισαγωγής/διαγραφής/εύρεσης, μπορούν να λειτουργήσουν και με `HP_info` στα οποία το attrName είναι `"name"`, `"surnname"` ή `"address"` (για όλα τα πιθανά πεδία δηλαδή).  
Αν για παράδειγμα όλες οι εγγραφές που βρίσκονται στο αρχείο έχουν το ίδιο πεδίο name, και κληθεί η `HP_GetAllEntries` έχοντας HP_info με `attrName` = `"name"`, θα εκτυπωθούν όλες οι εγγραφές.

### HT συναρτήσεις:
Επίσης έχουν υλοποιηθεί τα ζητούμενα της εκφώνησης.

Ένα HT αρχείο, ομοίως, αποθηκεύει την κεφαλίδα πληροφοριών του στο πρώτο του μπλοκ, το οποίο δεν περιέχει εγγραφές. Στην αρχή του μπλοκ αυτού αποθηκεύεται μια συμβολοσειρά `"hash"` που επιτρέπει να αναγνωρίσουμε αν πρόκειται για hash file ή όχι. Τα επόμενα μπλοκ, υλοποιούν τον πίνακα κατακερματισμού.

Ο HashTable είναι μια λίστα από blocks, τα οποία περιέχουν τα buckets. Ένα bucket είναι απλώς ο αριθμός του πρώτου του μπλοκ, αν είναι -1, τότε το bucket είναι άδειο (και δεν έχει μπει καμία εγγραφή εκεί ακόμα). Κάθε bucket στην ουσία μας υποδεικνύει μια λίστα από μπλοκ, της οποίας η λειτουργία ταυτίζεται με αυτή των HP συναρτήσεων.
Συνεπώς εσωτερικά σε κάθε bucket ακολουθούνται οι ίδιες πολιτικές σχήματος και εισαγωγών-διαγραφών.

Για hashing χρησιμοποιείται η SHA1 της `openssl`. Επιστρέφει ένα hashcode 20 bytes, από τα οποία κρατάμε τα αρχικά, και αφού κάνουμε modulo με τον αριθμό των buckets, έχουμε πλέον τον hashcode που θα χρησιμοποιηθεί. Σε απόδοση έχει καλά αποτελέσματα, για 5Κ εγγραφές με 10000 buckets, δεν φτιάχνεται κανένα overflow block. Για το αρχείο των 15K εγγραφών, το πρόγραμμα λειτουργεί για μέχρι 10000 buckets περίπου(πιθανώς η BF να μην έδινε πλέον άλλα μπλοκ)

Οι συναρτήσεις εισαγωγής και διαγραφής εκμεταλλεύονται το hashing για να κερδίσουν σε ταχύτητα. Αυτό σημαίνει ότι λειτουργούν μόνο για attrName = `"id"`. Όσον αφορά την ανάκτηση εγγραφών, έχουν υλοποιηθεί οι `HΤ_GetAllEntries` και `HT_GetUniqueEntry`. Η πρώτη λειτουργεί και για άλλα πεδία εκτός του id, αλλά δεν εκμεταλλέυεται το hashing (κάνει καθαρά σειριακή αναζήτηση όλου του αρχείου) και τυπώνει όσες εγγραφές συμφωνούν με το φίλτρο αναζήτησης. Η δεύτερη χρησιμοποιεί την ιδιότητα του hashing, και μειώνει αισθητά τον χρόνο εκτέλεσης. Λειτουργεί μόνο για πεδίο id σαν search key, και σταματάει μετά την εύρεση της πρώτης εγγραφής (η οποία είναι και μοναδική).

### SHT συναρτήσεις
Η λειτουργία τους έχει βασιστεί πάνω στις HT (να σημειωθεί εδώ ότι οι HT υλοποιήσεις είναι dependency των SHT). To client program εισάγει `SecondaryRecord` εγγραφές όπως ζητείται στην εκφώνηση, αλλά εσωτερικά οι SHT χρησιμοποιούν μια δομή `SHT_record` η οποία διατηρεί τις απαραίτητες πληροφορίες για κάθε εγγραφή SHT (surname και block ID).  
Είχε περιγραφεί στις διαλέξεις το εξής optimization, το οποίο και έχει υλοποιηθεί: Δεν υπάρχουν διπλότυπες εγγραφές στα SHT block, (δηλαδή ίδιο επίθετο και block ID). Όταν με την `SHT_SecondaryGetAllEntries` ζητηθούν οι εγγραφές με ένα συγκεκριμένο επίθετο και βρεθεί μια SHT εγγραφή για το επίθετο αυτό, θα τυπωθούν όλες οι εγγραφές του αντίστοιχου block του πρωτεύοντος αρχέιου με το συγκεκριμένο επίθετο. 

### HashStatistics:
Η `HashStatistics` τυπώνει για το αρχείο που δίνεται, στατιστικά για κάθε bucket ξεχωριστά, όπως ζητείται στην εκφώνηση, και στο τέλος εμφανίζει συγκεντρωτικά στατιστικά. Σε περιπτώσεις που τηρήθηκε καλή αναλογία εγγραφών/buckets, τα αποτελέσματα είναι πολύ ικανοποιητικά, με πολύ χαμηλό αριθμό overflow blocks. Η διαφορά σε χρόνο εκτέλεσης με την HP είναι παραπάνω από αισθητή: Η HP_main για να διαχειριστεί το αρχείο 15Κ εγγραφών χρειάζεται λίγα λεπτά, ενώ η SHT_main (ακόμα και με 10000 buckets, τα οποία είναι λίγα) ολοκληρώνεται σε λίγα δευτερόλεπτα.  
H `HashStatistics` λειτουργεί τόσο για αρχεία ΗΤ όσο και για SHT. Όπως είναι φυσικό, επειδή οι εγγραφές στα SHT blocks έχουν μικρότερο μέγεθος, το SHT αρχείο θα έχει πολύ λιγότερα overflow blocks από το αντίστοιχο HT πρωτεύον αρχείο.  
Επισημαίνεται ότι όταν ένα bucket δεν έχει αρχικοποιηθεί (δεν δείχνει σε κάποιο μπλοκ), δεν θα εμφανιστούν στατιστικά για αυτό.

### Διαφοροποιήσεις από την άσκηση 1:
- Η `HashStatistics` έχει μεταφερθεί στο `SHT.h` και `SHT.c`, επειδή πρέπει να μπορεί να ελέγξει αν το αρχείο που δίνεται είναι HT ή SHT (αν δεν είναι, το απορρίπτει).
- Στην Hash Function που χρησιμοποιούν οι HT συναρτήσεις υπήρχε λάθος λόγω του οποίου εμφανίζονταν κάποια errors από την Valgrind (δεν εμφανιζόταν πρόβλημα στην λειτουργία του προγράμματος), το οποίο έχει διορθωθεί.
- Η `HT_main.c` έχει αντικατασταθεί από την `SHT_main.c` στην οποία γίνεται επίδειξη της λειτουργίας των HT και SHT.

### Γενικές λεπτομέρειες
- Οι συναρτήσεις δεν αρχικοποιούν το επίπεδο μπλοκ, που σημαίνει ότι πρέπει η εκάστοτε main να καλέσει την `BF_Init()`
- Όπως προαναφέρθηκε, για το αρχείο των 15K εγγραφών, η `SHT_main` λειτουργεί για μέχρι 10000 buckets περίπου ανά αρχείο (10000 στο πρωτεύον και 10000 στο δευτερεύον), που πιθανώς σχετίζεται με την BF. Για 5Κ εγγραφές λειτουργεί κανονικά για 10Κ κάδους, ενώ για 10Κ εγγραφες επίσης λειτουργεί για 20Κ κάδους. Στις δύο αυτές περιπτώσεις, ο αριθμός overflow blocks ανά κάδο ήταν πολύ χαμηλός.
- Έχει γίνει εκτενής έλεγχος για memory leaks με valgrind, και δεν γίνεται κάποια αναφορά για leak. Γίνεται αναφορά για errors στις BF συναρτήσεις.

### Ανάπτυξη & Δοκιμές
Οι υλοποιήσεις αναπτύχθηκαν σε περιβάλλον Ubuntu 20.04, με Visual Studio Code. Έχουν δοκιμαστεί επιτυχώς και στα Ubuntu 16.04 του τμήματος.
