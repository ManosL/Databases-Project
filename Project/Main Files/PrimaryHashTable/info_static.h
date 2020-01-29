#ifndef _INFO_STATIC_
#define _INFO_STATIC_

typedef struct {
int type;
int fileDesc;
        /* αναγνωριστικός αριθμός ανοίγματος αρχείου από το επίπεδο block */
char attrType;     /* ο τύπος του πεδίου που είναι κλειδί για το συγκεκριμένο αρχείο, 'c' ή'i' */
char attrName[40];          /* το όνομα του πεδίου που είναι κλειδί για το συγκεκριμένο αρχείο */
int attrLength;           /* το μέγεθος του πεδίου που είναι κλειδί για το συγκεκριμένο αρχείο */
long int numBuckets;                 /* το πλήθος των “κάδων” του αρχείου κατακερματισμού */
} Static_info;

#endif