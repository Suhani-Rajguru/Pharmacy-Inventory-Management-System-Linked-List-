#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#define NAME_LEN 20
#define CONTACT_LEN 20
#define SUCCESS 1
#define FAILURE 0

typedef struct SupplierCount {
    int supplierID;
    int uniqueMedicationCount;
    struct SupplierCount* next;
} SuppCount;

typedef struct SuppTurnover {
    int supplierID;
    float totalCostSupplied;
    struct SuppTurnover* next;
} SuppT;

typedef struct ExpiryDate {
    int day;
    int month;
    int year;
}Date;

typedef struct Batch {
    char batchNumber[CONTACT_LEN];
    int quantityInStock;
    int totalSales;
    struct Batch *next;
}batch;

typedef struct Supplierinfo {
    int supplierID;
    char supplierName[NAME_LEN];
    char contactInfo[CONTACT_LEN];
    int quantitySupplied;
    struct Supplierinfo *next;//linked list ka initialize 
}sup;

typedef struct Medication{
    int medID;
    char mediName[NAME_LEN];
    int batchnumber;
    int totalQuantity;
    float priceperunit;
    int supplierCount;
    struct ExpiryDate Date;
    struct Supplierinfo* suppHead;//struct supplierinfo ke jagah sup direct likh sakte 
    struct Batch* batchHead;
    int reorderLevel;
    struct Medication *next;
}medicine;

void saveMedicationData(medicine *medHead);
medicine* loadMedicationData();
int addNewMedication(medicine **medHead);
int updateMedication(medicine* medHead);
int deleteMedicationByID(medicine** medHead);
int searchMedication(medicine *medHead);
void addSupplierToMedication(medicine* medHead);
void sales_tracking(medicine *medHead);
void updateSupplierInMedication(medicine* medHead);
void supplierManagement(medicine *medHead);
void searchSupplierInMedication(medicine* medHead);
void stockAlert(medicine* current);
void checkExpirationDates(medicine *medHead);
medicine* sort_medication_by_expiry_date(medicine* medHead);
SuppCount* computeUniqueMedicationCount(medicine* medHead);
void Top_10_rounderSupplier(medicine* medHead);
void Top_10_LargestTurnover(medicine* medHead);
SuppT* computeSupplierTurnover(medicine* medHead);

// Function to save medication data to a file
void saveMedicationData(medicine* medHead) {
    FILE* fp = fopen("medication_data.txt", "w");
    if (fp == NULL) {
        printf("Error opening file for writing medication data.\n");
        return;
    }

    medicine* current = medHead;
    while (current != NULL) {
        fprintf(fp, "%d %s %f %d %d %d %d %d\n",
            current->medID, current->mediName, current->priceperunit,
            current->reorderLevel, current->totalQuantity,
            current->Date.day, current->Date.month, current->Date.year);
        
        int batchCount = 0;
        batch* tempBatch = current->batchHead;
        while (tempBatch != NULL) {
            batchCount++;
            tempBatch = tempBatch->next;
        }
        fprintf(fp, "%d\n", batchCount);
        
        batch* currentBatch = current->batchHead;
        while (currentBatch != NULL) {
            fprintf(fp, "%s %d %d\n", currentBatch->batchNumber, currentBatch->quantityInStock, currentBatch->totalSales);
            currentBatch = currentBatch->next;
        }
        
        int suppCount = 0;
        sup* tempSupp = current->suppHead;
        while (tempSupp != NULL) {
            suppCount++;
            tempSupp = tempSupp->next;
        }
        fprintf(fp, "%d\n", suppCount);
        
        sup* currentSupp = current->suppHead;
        while (currentSupp != NULL) {
            fprintf(fp, "%d %s %s %d\n",
                currentSupp->supplierID, currentSupp->supplierName, currentSupp->contactInfo, currentSupp->quantitySupplied);
            currentSupp = currentSupp->next;
        }
        
        current = current->next;
    }
    fclose(fp);
    printf("Medication data saved successfully.\n");
}

// Function to load medication data from file
medicine* loadMedicationData() {
    FILE* fp = fopen("medication_data.txt", "r");
    if (fp == NULL) {
        printf("No existing medication data found.\n");
        return NULL;
    }

    medicine* medHead = NULL;
    medicine* prev = NULL;

    while (!feof(fp)) {
        medicine* current = (medicine*)malloc(sizeof(medicine));
        if (current == NULL) {
            printf("Memory allocation failed while loading data.\n");
            break;
        }

        current->batchHead = NULL;
        current->suppHead = NULL;
        current->next = NULL;

        if (fscanf(fp, "%d %s %f %d %d %d %d %d", &current->medID, current->mediName,
            &current->priceperunit, &current->reorderLevel, &current->totalQuantity,
            &current->Date.day, &current->Date.month, &current->Date.year) != 8) {
            free(current);
            break;
        }

        int batchCount;
        fscanf(fp, "%d", &batchCount);
        for (int i = 0; i < batchCount; i++) {
            batch* newBatch = (batch*)malloc(sizeof(batch));
            fscanf(fp, "%s %d %d", newBatch->batchNumber, &newBatch->quantityInStock, &newBatch->totalSales);
            newBatch->next = current->batchHead;
            current->batchHead = newBatch;
        }

        int suppCount;
        fscanf(fp, "%d", &suppCount);
        for (int i = 0; i < suppCount; i++) {
            sup* newSupp = (sup*)malloc(sizeof(sup));
            fscanf(fp, "%d %s %s %d", &newSupp->supplierID, newSupp->supplierName, newSupp->contactInfo, &newSupp->quantitySupplied);
            newSupp->next = current->suppHead;
            current->suppHead = newSupp;
        }

        if (medHead == NULL) {
            medHead = current;
        } else {
            prev->next = current;
        }
        prev = current;
    }
    fclose(fp);
    printf("Medication data loaded successfully.\n");
    return medHead;
}

// Function to free all allocated memory
void freeAllMemory(medicine* medHead) {
    medicine* current = medHead;
    while (current != NULL) {
        batch* currentBatch = current->batchHead;
        while (currentBatch != NULL) {
            batch* tempBatch = currentBatch;
            currentBatch = currentBatch->next;
            free(tempBatch);
        }
        sup* currentSupp = current->suppHead;
        while (currentSupp != NULL) {
            sup* tempSupp = currentSupp;
            currentSupp = currentSupp->next;
            free(tempSupp);
        }
        medicine* temp = current;
        current = current->next;
        free(temp);
    }
}

//top 10 largest turnover supplier
void addOrUpdateSupplierTurnover(SuppT** head, int supplierid, float totalCost) {
    SuppT* temp = *head;

    while (temp != NULL) {
        if (temp->supplierID == supplierid) {
            temp->totalCostSupplied += totalCost;  // Add total cost if found
            return;
        }
        temp = temp->next;
    }

    // If supplier not found, create a new node
    SuppT* newNode = (SuppT*)malloc(sizeof(SuppT));
    newNode->supplierID = supplierid;
    newNode->totalCostSupplied = totalCost;
    newNode->next = *head;
    *head = newNode;
}

SuppT* computeSupplierTurnover(medicine* medHead) {
    SuppT* suppTurnoverList = NULL;
    medicine *current=medHead;

    while (current!= NULL) {
        sup* supplier = current->suppHead; // Get supplier list of current medicine

        while (supplier != NULL) {
            float totalCost = supplier->quantitySupplied * current->priceperunit;
            addOrUpdateSupplierTurnover(&suppTurnoverList, supplier->supplierID, totalCost);
            supplier = supplier->next;
        }

        current= current->next;
    }

    return suppTurnoverList;
}

SuppT* merge1(SuppT* list1 ,SuppT* list2)
{
    SuppT *result,*ptr1,*ptr2,*tail;
    ptr1=list1;
    ptr2=list2;
    if(list1->totalCostSupplied > list2->totalCostSupplied)
    {
        tail=result=list1;
        ptr1=ptr1->next;
    }
    else
    {
        tail=result=list2;
        ptr2=ptr2->next;
    }
    while(ptr1!=NULL && ptr2!=NULL)
    {
        if(ptr1->totalCostSupplied > ptr2->totalCostSupplied)
        {
            tail->next=ptr1;
            tail=tail->next;
            ptr1=ptr1->next;
        }
        else
        {
            tail->next=ptr2;
            tail=tail->next;
            ptr2=ptr2->next;
        }
    }
    if(ptr1!=NULL)
    {
        tail->next=ptr1;
    }
    else
    {
        tail->next=ptr2;
    }
    return result;
}

SuppT* divide1(SuppT* lptr)
{
    SuppT *fast,*slow,*nptr;
    slow=lptr;
    fast=lptr->next->next;
    while(fast!=NULL)
    {
        slow=slow->next;
        fast=fast->next;
        if(fast!=NULL)
        {
            fast=fast->next;
        }
    }
    nptr=slow->next;
    slow->next=NULL;
    return nptr;
}

SuppT* mergesort1(SuppT* supphead)
{
    SuppT *nptr,*list_ptr;
    list_ptr=supphead;
    if(supphead!=NULL && supphead->next!=NULL)
    {
        nptr=divide1(list_ptr);//second head
        list_ptr=mergesort1(list_ptr);
        nptr=mergesort1(nptr);
        supphead=merge1(list_ptr,nptr);
    }
    return supphead;
}

void Top_10_LargestTurnover(medicine* medHead) {
    SuppT* turnoverHead = computeSupplierTurnover(medHead);
    turnoverHead = mergesort1(turnoverHead);

    printf("\nTop 10 Largest Turnover Suppliers:\n");
    SuppT* temp = turnoverHead;
    int count = 0;

    while (temp != NULL && count < 10) {
        printf("Supplier ID: %d, Total Turnover: %.2f\n", temp->supplierID, temp->totalCostSupplied);
        temp = temp->next;
        count++;
    }

    printf("\nHighest Turnover Supplier:\n");
    printf("Supplier ID: %d, Total Turnover: %.2f\n", turnoverHead->supplierID, turnoverHead->totalCostSupplied);

    // Free memory
    while (turnoverHead != NULL) {
        SuppT* toFree = turnoverHead;
        turnoverHead = turnoverHead->next;
        free(toFree);
        toFree=NULL;
    }
}

//top 10 all rounder suppliers
void addOrUpdateSupplierCount(SuppCount** head, int supplierid) {
    SuppCount* temp = *head;
    
    // Check if supplier already exists in the list
    while (temp != NULL) {
        if (temp->supplierID == supplierid) {
            temp->uniqueMedicationCount++; // Increment count if found
            return;
        }
        temp = temp->next;
    }

    // If not found, create a new node
    SuppCount* newNode = (SuppCount*)malloc(sizeof(SuppCount));
    newNode->supplierID = supplierid;
    newNode->uniqueMedicationCount = 1;
    newNode->next = *head;
    *head = newNode;
}

SuppCount* computeUniqueMedicationCount(medicine* medHead) {
    SuppCount* suppCountList = NULL;
    medicine *current=medHead;

    // Traverse all medications
    while (current!= NULL) {
        sup* supplier = current->suppHead;

        // Traverse supplier list of the current medication
        while (supplier != NULL) {
            addOrUpdateSupplierCount(&suppCountList, supplier->supplierID);
            supplier = supplier->next;
        }

        current = current->next;
    }

    return suppCountList;
}

SuppCount* merge2(SuppCount* list1 ,SuppCount* list2)
{
    SuppCount *result,*ptr1,*ptr2,*tail;
    ptr1=list1;
    ptr2=list2;
    if(list1->uniqueMedicationCount > list2->uniqueMedicationCount)
    {
        tail=result=list1;
        ptr1=ptr1->next;
    }
    else
    {
        tail=result=list2;
        ptr2=ptr2->next;
    }
    while(ptr1!=NULL && ptr2!=NULL)
    {
        if(ptr1->uniqueMedicationCount > ptr2->uniqueMedicationCount)
        {
            tail->next=ptr1;
            tail=tail->next;
            ptr1=ptr1->next;
        }
        else
        {
            tail->next=ptr2;
            tail=tail->next;
            ptr2=ptr2->next;
        }
    }
    if(ptr1!=NULL)
    {
        tail->next=ptr1;
    }
    else
    {
        tail->next=ptr2;
    }
    return result;
}

SuppCount* divide2(SuppCount* lptr)
{
    SuppCount *fast,*slow,*nptr;
    slow=lptr;
    fast=lptr->next->next;
    while(fast!=NULL)
    {
        slow=slow->next;
        fast=fast->next;
        if(fast!=NULL)
        {
            fast=fast->next;
        }
    }
    nptr=slow->next;
    slow->next=NULL;
    return nptr;
}

SuppCount* mergesort2(SuppCount* supphead)
{
    SuppCount *nptr,*list_ptr;
    list_ptr=supphead;
    if(supphead!=NULL && supphead->next!=NULL)
    {
        nptr=divide2(list_ptr);//second head
        list_ptr=mergesort2(list_ptr);
        nptr=mergesort2(nptr);
        supphead=merge2(list_ptr,nptr);
    }
    return supphead;
}

void Top_10_rounderSupplier(medicine* medHead) {
    SuppCount* supphead = computeUniqueMedicationCount(medHead);
    // Sort the supplier list using Merge Sort
    supphead=mergesort2(supphead);

    printf("\nTop 10 All-Rounder Suppliers:\n");
    SuppCount* bestSupplier = supphead; // First supplier in sorted list is the best

    int count = 0;
    SuppCount* temp = supphead;
    while (temp != NULL && count < 10) {
        printf("Supplier ID: %d, Unique Medications Supplied: %d\n", temp->supplierID, temp->uniqueMedicationCount);
        temp = temp->next;
        count++;
    }

    printf("\nBest All-Rounder Supplier:\n");
    printf("Supplier ID: %d, Unique Medications Supplied: %d\n", bestSupplier->supplierID, bestSupplier->uniqueMedicationCount);

    // Free the supplier count list
    while (supphead!= NULL) {
        SuppCount* toFree = supphead;
        supphead = supphead->next;
        free(toFree);
        toFree=NULL;
    }
}


int addNewMedication(medicine **medHead){
    medicine* newMedicine=(medicine*)malloc(sizeof(medicine));
    if(newMedicine==NULL)
    {
        printf("memory allocation for medicine failed.\n");
        return FAILURE;
    }
    printf("Enter Medication ID: ");
    scanf("%d", &newMedicine->medID);

    printf("Enter Medication Name: ");
    scanf("%s", &newMedicine->mediName);

    printf("Enter Price Per Unit: ");
    scanf("%f", &newMedicine->priceperunit);

    printf("Enter Reorder Level: ");
    scanf("%d", &newMedicine->reorderLevel);

    printf("Enter Expiration Date (DD MM YYYY) for medicine %d: ", newMedicine->medID);
        scanf("%d %d %d", &newMedicine->Date.day, 
                         &newMedicine->Date.month, 
                         &newMedicine->Date.year);

    
    newMedicine->batchHead=NULL;
    newMedicine->suppHead=NULL;
    newMedicine->totalQuantity=0;

    int batchcount;
    printf("enter the number of batches: ");
    scanf("%d",&batchcount);

    for(int i=0;i<batchcount;i++)
    {
        batch* newBatch=(batch*)malloc(sizeof(batch));
        if(newBatch==NULL)
        {
            printf("memory allocation for batch failed.\n");
            return FAILURE;
        }
        printf("Enter Batch Number for Batch %d: ", i + 1);
        scanf(" %s",newBatch->batchNumber);

        printf("Enter Quantity in Stock for Batch %d: ", i + 1);
        scanf("%d", &newBatch->quantityInStock);

        newBatch->next=newMedicine->batchHead;
        newMedicine->batchHead=newBatch;
        newBatch->totalSales=0;

        newMedicine->totalQuantity=newMedicine->totalQuantity+newBatch->quantityInStock;
    }

    int suppliercount;
    printf("Enter number of suppliers: ");
    scanf("%d", &suppliercount);

    for (int i = 0; i < suppliercount; i++) 
    {
        sup* newsupp=(sup*)malloc(sizeof(sup));
        if(newsupp==NULL)
        {
            printf("memory allocation failed for supplier.\n");
            return FAILURE;
        }
        printf("Enter Supplier ID for Supplier %d: ", i + 1);
        scanf("%d", &newsupp->supplierID);

        printf("Enter Supplier Name for Supplier %d: ", i + 1);
        scanf("%s", newsupp->supplierName);

        printf("Enter Contact Info for Supplier %d: ", i + 1);
        scanf("%s", newsupp->contactInfo);

        printf("Enter Quantity Supplied by Supplier %d: ", i + 1);
        scanf("%d", &newsupp->quantitySupplied);

        newsupp->next=newMedicine->suppHead;
        newMedicine->suppHead=newsupp; 
    }

    //inserting new medication at the beginning
    newMedicine->next=*medHead;
    *medHead=newMedicine;

    printf("New medication added successfully ");
    //saveMedicationData(*medHead);
    return SUCCESS;
}

int updateMedication(medicine* medHead) 
{
    int status_code = SUCCESS;
    int id;
    
    printf("Enter the medication ID you want to update: ");
    scanf("%d", &id);
    
    medicine* current = medHead; 

    while (current != NULL && current->medID!=id) 
    {
        current = current->next;
    }

    if (current!=NULL) 
    {
        int option;
        float newPrice;
        int newReorderLevel;
        int newTotalQuantity;
        char newSupplierName[NAME_LEN];
        char newContactInfo[CONTACT_LEN];

        
        printf("Enter 1 to modify price\n");
        scanf("%d", &option);
        if (option == 1) 
        {
            printf("Enter new price: ");
            scanf("%f", &newPrice);
            current->priceperunit = newPrice;
        }

        printf("Enter 2 to modify reorder level\n");
        scanf("%d", &option);
        if (option == 2) 
        {
            printf("Enter new reorder level: ");
            scanf("%d", &newReorderLevel);
            current->reorderLevel = newReorderLevel;
        }

        printf("Enter 3 to modify total quantity\n");
        scanf("%d", &option);
        if (option == 3) 
        {
            printf("Enter new total quantity: ");
            scanf("%d", &newTotalQuantity);
            current->totalQuantity = newTotalQuantity;
        }

        printf("Enter 4 to modify supplier info\n");
        scanf("%d", &option);
        if (option == 4) 
        {
            sup* supplier = current->suppHead;

            while (supplier != NULL) 
            {
                printf("Updating Supplier (ID: %d)\n", supplier->supplierID);

                printf("Enter updated Supplier ID: ");
                scanf("%d", &supplier->supplierID);
                
                printf("Enter updated Supplier Name: ");
                scanf("%s", newSupplierName);
                strcpy(supplier->supplierName, newSupplierName);
                
                printf("Enter updated Contact Info: ");
                scanf("%s", newContactInfo);
                strcpy(supplier->contactInfo, newContactInfo);
                
                supplier = supplier->next;  
            }
        }

        printf("Medication details updated successfully.\n");

    } 
    else 
    {
        printf("Medication with ID %d not found.\n", id);
        status_code = FAILURE;
    }

    return status_code;
}

int deleteMedicationByID(medicine** medHead) 
{
    int status_code=SUCCESS;
    int id;

    printf("Enter the medication ID you want to delete: ");
    scanf("%d", &id);

    medicine* current=*medHead;
    medicine* prev=NULL;

    while (current != NULL && current->medID != id) 
    {
        prev=current;
        current=current->next;
    }

    if(current!=NULL)
    {
       
        if(prev==NULL)//deleting head
        {
            *medHead=current->next;
        }
        else
        {
            prev->next = current->next;
        }
        batch* batchTemp;
        while (current->batchHead != NULL)
        {
            batchTemp = current->batchHead;
            current->batchHead = current->batchHead->next;
            free(batchTemp);
            batchTemp=NULL;
        }

        sup* suppTemp;
        while (current->suppHead!= NULL)
        {
            suppTemp = current->suppHead;
            current->suppHead=current->suppHead->next;
            free(suppTemp);
            suppTemp=NULL;
        }
        free(current);
        current=NULL;
        printf("Medication with ID %d deleted successfully.\n", id);  
    }
    else
    {
        printf("Medication with ID %d not found.\n", id);
        status_code = FAILURE;
    }

    return status_code;
}

//search
int searchMedication(medicine *medHead) 
{   
    loadMedicationData(medHead);
    int statuscode=SUCCESS;
    int Option;
    printf("Enter 1 to search by Medication ID,\n enter 2 to search by Medication Name,\n enter 3 to search by Supplier ID ");
    printf("Enter your option: ");
    scanf("%d", &Option);

    switch (Option) {
        case 1: {
            int id;
            printf("Enter Medication ID: ");
            scanf("%d", &id);

            medicine *current = medHead; 
            while(current!=NULL && current->medID!=id){
                current=current->next;
            }
            if(current !=NULL){
                printf("Medication found:\n");
                    printf("ID: %d\nName: %s,\nPrice per Unit: %f,\nReorder Level: %d,\n",
                           current->medID, current->mediName,
                           current->priceperunit,current->reorderLevel);

                    printf(" Expiration Date:%d %d %d\n",current->Date.day,current->Date.month,current->Date.year);
                    printf("Batches of medicine and quantity in stock according to batch no.");
                    batch *batchtemp=current->batchHead;
                    while(batchtemp !=NULL){
                    printf("quantity in stock of medicine in batch no. :%s is %d\n",
                    batchtemp->batchNumber,batchtemp->quantityInStock);
                    batchtemp=batchtemp->next;
                }
            }
            else{
                printf("medication not found");
                statuscode=FAILURE;
            }
            break;
        }

        case 2: {
            char name[NAME_LEN];
            printf("Enter Medication Name: ");
            scanf("%s",name);
            
            int found1=0;
            medicine *current=medHead;
            while (current !=NULL){
                if(strcmp(current->mediName,name)==0){
                    printf("Medication found.\n");
                    printf("ID: %d,\nName: %s,\nPrice per Unit: %f,\nReorder Level: %d,\n",
                           current->medID,current->mediName, 
                           current->priceperunit,current->reorderLevel);
                    printf(" Expiration Date:%d %d %d\n",current->Date.day,current->Date.month,current->Date.year);
                    printf("Batches of medicine and quantity in stock according to batch no.");
                    batch* batchtemp=current->batchHead;
                    while(batchtemp !=NULL){
                    printf("quantity in stock of medicine in batch no. :%s is %d\n",
                    batchtemp->batchNumber,batchtemp->quantityInStock);
                    batchtemp=batchtemp->next;
                    }
                    found1=1;
                }
                current=current->next;
            }
            if (!found1){
                printf("No medications found with the name %s.\n", name);
                statuscode=FAILURE;
            }
            break;
        }

        case 3: {
            int suppID;
            printf("Enter Supplier ID: ");
            scanf("%d",&suppID);
            int found2=0;
            medicine *current = medHead;
                    while(current != NULL){
                        sup *suptemp=current->suppHead;
                        while(suptemp!=NULL){
                            if(suptemp->supplierID==suppID){
                                printf("Supplier found: %d\n", suptemp->supplierID);
                                printf("Medication Name: %s,\nPrice per Unit: %f,Reorder Level: %d,\n",
                                current->mediName,current->priceperunit,current->reorderLevel);
                                printf(" Expiration Date:%d %d %d\n",current->Date.day,current->Date.month,current->Date.year);
                                found2=1;
                            }
                            suptemp=suptemp->next;
                        }
                        current=current->next;
                    }
                    if(! found2 ){
                        printf("No medications found for supplier %d.\n", suppID);
                    }
                    break;
                }
                default:
            printf("Invalid search option!\n");
            break;
        }    
}
    
void addSupplierToMedication(medicine* medHead) {
    int id,suppID;
    printf("Enter Medication ID to add a supplier: ");
    scanf("%d", &id);

    medicine* current=medHead;
    while (current!= NULL &&current->medID!=id) 
    {
        current=current->next;
    }
    if(current!=NULL)
    {
        printf("Enter Supplier ID: ");
        scanf("%d",&suppID);
        //check if supplier already exists for this medication
        sup* temp=current->suppHead;
        while(temp!=NULL&&temp->supplierID!=suppID)
        {
            temp=temp->next;
        }
        if(temp==NULL)
        {
            sup* newSupplier = (sup*)malloc(sizeof(sup));
            newSupplier->supplierID=suppID;
            printf("Enter Supplier Name: ");
            scanf("%s", newSupplier->supplierName);
            printf("Enter Supplier Contact Info: ");
            scanf("%s", newSupplier->contactInfo);
            printf("Enter Quantity Supplied: ");
            scanf("%d", &newSupplier->quantitySupplied);
    
            newSupplier->next = current->suppHead;
            current->suppHead = newSupplier; 
            printf("Supplier added successfully to Medication ID: %d.\n", id); 
        }
        else
        {
            printf("Supplier ID %d is already supplying this medication. \n", suppID);
        }
        
    }
    else
    {
        printf("Medication not found.\n");
    }
}

void updateSupplierInMedication(medicine* medHead) 
{
    int id;
    printf("Enter Medication ID to update a supplier: ");
    scanf("%d", &id);
    
    medicine* med = medHead;
    while (med!=NULL && med->medID!=id) 
    {
        med=med->next;
    }
    if(med!=NULL)
    {   
        int suppID;
        printf("Enter Supplier ID to update: ");
        scanf("%d", &suppID);
        sup* supp=med->suppHead;
        while (supp != NULL && supp->supplierID!=suppID) 
        {
           supp = supp->next;
        }
        if(supp!=NULL)
        {
            printf("Enter updated Supplier Name: ");
            scanf("%s", supp->supplierName);
            printf("Enter updated Contact Info: ");
            scanf("%s", supp->contactInfo);
            printf("Enter updated Quantity Supplied: ");
            scanf("%d", &supp->quantitySupplied);
            printf("Supplier updated successfully for Medication ID: %d.\n", id);
        }
        else
        {
           printf("supplier not found.\n");
        }
    }
    else{
        printf("medication not found.\n");
    }
}

void searchSupplierInMedication(medicine* medHead) 
{
    int id, suppID;
    printf("Enter Medication ID to search for a supplier: ");
    scanf("%d", &id);

    medicine* current=medHead;
    while(current!=NULL && current->medID!=id)
    {
        current=current->next;
    }
    if(current!=NULL)
    {
        printf("Enter Supplier ID to search: ");
        scanf("%d", &suppID);
        sup* suppTemp=current->suppHead;
        while(suppTemp!=NULL && suppTemp->supplierID!=suppID)
        {
            suppTemp=suppTemp->next;
        }
        if(suppTemp!=NULL)
        {
            printf("Supplier ID: %d\n",suppTemp->supplierID);
            printf("Supplier Name: %s\n",suppTemp->supplierName);
            printf("Contact Info: %s\n",suppTemp->contactInfo);
            printf("Quantity Supplied: %d\n",suppTemp->quantitySupplied);
        }
        else
        {
            printf("Supplier not found.\n");
        }
    }
    else
    {
        printf("Medication not found.\n");
    }

}


void supplierManagement(medicine *medHead) 
{
    int choice;
    do {
        printf("Supplier Management Menu:\n");
        printf("1. Add Supplier\n");
        printf("2. Update Supplier\n");
        printf("3. Search Supplier\n");
        printf("4. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                addSupplierToMedication(medHead);
                break;
            case 2:
                updateSupplierInMedication(medHead);
                break;
            case 3:
                searchSupplierInMedication(medHead);
                break;
            case 4:
                printf("Exiting...\n");
                break;
            default:
                printf("Invalid choice!\n");
        }
    } while (choice != 4);
}

void sales_tracking(medicine *medHead) 
{   
    int quantity, medID;
    printf("Enter the ID of the medicine you want to buy: \n");
    scanf("%d",&medID);
    printf("Enter the quantity: \n");
    scanf("%d",&quantity);

    int remainingQty=quantity;
    int batchQty=0;

    medicine* current=medHead;
    while (current!=NULL && current->medID!=medID) 
    {
        current = current->next;
    }
    if (current != NULL) 
    {
        printf("Medication found: %d\n", current->medID);

        batch *batchTemp = current->batchHead;
        while (batchTemp!=NULL && remainingQty>0) 
        {
            batchQty = batchTemp->quantityInStock;
            if (remainingQty <= batchQty) 
            {
                batchTemp->quantityInStock -= remainingQty;
                batchTemp->totalSales += remainingQty;
                current->totalQuantity -= remainingQty;
                printf("Sale completed for %d units from batch %s.\n", remainingQty, batchTemp->batchNumber);
                printf("Now total quantity of ID %d is: %d\n", current->medID, current->totalQuantity);
                stockAlert(current);
                remainingQty = 0;
            } 
            else 
            {
                printf("Depleting batch %s: %d units sold.\n", batchTemp->batchNumber, batchQty);
                remainingQty -= batchQty;
                batchTemp->totalSales += batchQty;
                batchTemp->quantityInStock = 0;
                current->totalQuantity -= batchQty;
                printf("Now total quantity of ID %d is: %d\n", current->medID, current->totalQuantity);
                stockAlert(current);
            }
            batchTemp = batchTemp->next;
        }

        if (remainingQty > 0) 
        {
            printf("Error: Not enough stock to fulfill %d units. Only partial order fulfilled.\n", quantity);
        }
    }   
    else 
    {
        printf("Error: Medication with ID %d not found.\n", medID);
    }
}

void stockAlert(medicine* current)
{
    printf("checking stock levels....\n");
    if(current->totalQuantity<=current->reorderLevel)
    {
        printf("Alert : Medication '% s'(ID:%d)needs restocking! \n",current->mediName,current->medID);
        printf("current stock : %d,\n Reorder Level : %d\n",current->totalQuantity,current->reorderLevel);
    }
    else
    {
        printf("Quantity in stock is sufficient");
    }
}

int daysInMonth(int month) 
{
    int daysPerMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return daysPerMonth[month - 1];
}

int calculateDateDifference(Date date1,Date date2) {
    int days1 = 0, days2 = 0;
    days1 = date1.year * 365 + date1.day;
    for (int i = 1; i < date1.month; i++) 
    {
        days1 += daysInMonth(i);
    }
    days2 = date2.year * 365 + date2.day;
    for (int i = 1; i < date2.month; i++) 
    {
        days2 += daysInMonth(i);
    }
    return days2 - days1;
}

void checkExpirationDates(medicine *medHead) 
{
    struct ExpiryDate today;
    medicine *current = medHead;
    printf("Enter today's date (DD MM YYYY): ");
    scanf("%d %d %d", &today.day, &today.month, &today.year);
    while (current != NULL) 
    {
        int daysToExpiry = calculateDateDifference(today, current->Date);
        if (daysToExpiry > 0 && daysToExpiry <= 30) 
        {
            printf("ALERT: Medication ID %d (%s) is expiring in %d days!\n",
                   current->medID, current->mediName, daysToExpiry);
        } 
        else if (daysToExpiry <= 0) 
        {
            printf("EXPIRED: Medication ID %d (%s) has already expired!\n",
                   current->medID, current->mediName);
        }
        current = current->next; 
    }
}

int compareExpiryDates(Date date1,Date date2) 
{
    if (date1.year != date2.year) {
        return (date1.year < date2.year) ? -1 : 1;
    } else if (date1.month != date2.month) {
        return (date1.month < date2.month) ? -1 : 1;
    } else {
        return (date1.day < date2.day) ? -1 : (date1.day > date2.day);
    }
}

medicine* merge(medicine* list1 ,medicine* list2)
{
    medicine *result,*ptr1,*ptr2,*tail;
    ptr1=list1;
    ptr2=list2;
    if((compareExpiryDates(list1->Date,list2->Date) )<0)
    {
        tail=result=list1;
        ptr1=ptr1->next;
    }
    else
    {
        tail=result=list2;
        ptr2=ptr2->next;
    }
    while(ptr1!=NULL && ptr2!=NULL)
    {
        if((compareExpiryDates(ptr1->Date,ptr2->Date) )<0)
        {
            tail->next=ptr1;
            tail=tail->next;
            ptr1=ptr1->next;
        }
        else
        {
            tail->next=ptr2;
            tail=tail->next;
            ptr2=ptr2->next;
        }
    }
    if(ptr1!=NULL)
    {
        tail->next=ptr1;
    }
    else
    {
        tail->next=ptr2;
    }
    return result;
}

medicine* divide(medicine* lptr)
{
    medicine *fast,*slow,*nptr;
    slow=lptr;
    fast=lptr->next->next;
    while(fast!=NULL)
    {
        slow=slow->next;
        fast=fast->next;
        if(fast!=NULL)
        {
            fast=fast->next;
        }
    }
    nptr=slow->next;
    slow->next=NULL;
    return nptr;
}

medicine* sort_medication_by_expiry_date(medicine* medHead)
{
    medicine *nptr,*list_ptr;
    list_ptr=medHead;
    if(medHead!=NULL && medHead->next!=NULL)
    {
        nptr=divide(list_ptr);//second head
        list_ptr=sort_medication_by_expiry_date(list_ptr);
        nptr=sort_medication_by_expiry_date(nptr);
        medHead=merge(list_ptr,nptr);
    }
    return medHead;
}


int main()
{
    //medicine *medi_list=NULL; //initialisation of medi list
    medicine* medi_list = loadMedicationData();
    sort_medication_by_expiry_date(medi_list);
    int choice;

    do {
        printf("\n----------------------------------------\n");
        printf("Main Menu:\n");
        printf("1. Add New Medication: Add new medications with batch and supplier details.\n");
        printf("2. Update Medication Details: Modify the details of an existing medication.\n");
        printf("3. Delete Medication: Remove a medication from the inventory.\n");
        printf("4. Search Medication: Look up medications by ID, name, or supplier.\n");
        printf("5. Check Expiration Dates: View and manage medications nearing expiry.\n");
        printf("6. Sales Tracking: Record sales and update inventory levels accordingly.\n");
        printf("7. Supplier Management: Add, update, or search for supplier information.\n");
        printf("8. Find All-rounder Suppliers: Identify suppliers offering the most unique medications.\n");
        printf("9. Find Suppliers with Largest Turnover: Identify suppliers with the highest turnover.\n");
        printf("0. Exit: Quit the system.\n");
        printf("----------------------------------------\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch(choice) {
            case 1:
                printf("\nYou chose to add a new medication.\n");
                addNewMedication(&medi_list);
                break;
            case 2:
                printf("\nYou chose to update existing medication details.\n");
                updateMedication(medi_list);
                break;
            case 3:
                printf("\nYou chose to delete a medication.\n");
                deleteMedicationByID(&medi_list);
                break;
            case 4:
                printf("\nYou chose to search for a medication.\n");
                searchMedication(medi_list); 
                break;
            case 5:
                printf("\nChecking expiration dates of medications...\n");
                checkExpirationDates(medi_list);
                break;
            case 6:
                printf("\nYou chose to track sales and update inventory.\n");
                sales_tracking(medi_list);
                break;
            case 7:
                printf("\nManaging supplier information...\n");
                supplierManagement(medi_list);
                break;
            case 8:
                printf("\nFinding top all-rounder suppliers...\n");
                Top_10_rounderSupplier(medi_list);
                break;
            case 9:
                printf("\nFinding suppliers with the largest turnover...\n");
                Top_10_LargestTurnover(medi_list);
                break;
            case 0:
                printf("\nExiting the system. Thank you for using the Pharmacy Inventory Management System. Goodbye!\n");
                saveMedicationData(medi_list);
                freeAllMemory(medi_list); 
                break;
            default:
                printf("\nInvalid choice. Please enter a number between 0 and 9.\n");
        }
    } while (choice != 0);
    return 0;
}

