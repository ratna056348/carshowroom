#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#define SIZE 2000
#define HASH_SIZE 101
#define MONTHS 12
#define ORDER 5
#define GROWTH_RATE 0.10 
#define DEFAULT_SALES 5
#define MAX 100

typedef enum
{
    FAILURE,
    SUCCESS
} status_code;

typedef enum
{
    FALSE,
    TRUE
} boolean;

typedef struct customer
{
    char customer_id[SIZE];
    char customer_name[SIZE];
    char mobile_no[11];
    char customer_address[SIZE];
    char registration_number[SIZE];
    char sold_car_VIN[SIZE];
} customer;

typedef struct BTree_Customer_node
{
    customer customer_data[ORDER - 1];
    struct BTree_Customer_node *children[ORDER];
    int customer_num_keys;
    boolean isleaf;
} customer_node;

typedef struct customer_root_tag
{
    char sales_person_id[SIZE];
    customer_node *customer_children;
} customer_root;

typedef struct car
{
    char car_VIN[SIZE];
    char car_name[SIZE];
    char car_type[SIZE];
    char car_colour[SIZE];
    char fuel_type[SIZE];
    float car_price;
    char car_customer_ID[SIZE];
    char car_sales_person_ID[SIZE];
    char purchase_date[SIZE];
} car;

typedef struct BTree_car_node
{
    car car_data[ORDER - 1];
    struct BTree_car_node *children[ORDER];
    int car_num_keys;
    boolean isleaf;
} car_node;

typedef struct car_root_tag
{
    char unique_id[SIZE];
    car_node *car_children;
} car_root;

typedef struct sold_car_tag
{
    char VIN[SIZE];
    char Payment_Type[SIZE];
    int loan_period;
    char purchase_date[SIZE];
} sold_car_info;

typedef struct sold_car_node_tag
{
    sold_car_info sold_car[ORDER - 1];
    struct sold_car_node_tag *children[ORDER];
    int sold_car_num_keys;
    boolean isleaf;
} sold_car_node;

typedef struct sales_person
{
    char sales_person_ID[SIZE];
    char sales_person_name[SIZE];
    float sales_target;
    float sales_achived;
    float commission_per_sale;
    customer_root *customer_tree_root;
    sold_car_node *sold_car_root;
} sales_person;

typedef struct BTree_sales_person_node
{
    sales_person sales_person_data[ORDER - 1];
    struct BTree_sales_person_node *children[ORDER];
    int sales_person_num_keys;
    boolean isleaf;
} sales_person_node;

typedef struct sales_person_root_tag
{
    char showroom_id[SIZE];
    sales_person_node *sales_person_children;
} sales_person_root;

typedef struct showroom
{
    char showroom_id[SIZE];
    car_root *sold_cars_root;
    car_root *avialable_cars_root;
    sales_person_root *salespersons_root;
    int sold_cars;
    int available_cars;
} showroom;

// Create a new node
car_node *create_car_node(boolean is_leaf)
{
    car_node *node = (car_node *)malloc(sizeof(car_node));
    node->isleaf = is_leaf;
    node->car_num_keys = 0;
    for (int i = 0; i < ORDER; i++)
    {
        node->children[i] = NULL;
    }
    return node;
}

// Traverse the tree (in-order)
void traverse_car_tree(car_node *root)
{
    
    if (root != NULL)
    {
        for (int i = 0; i < root->car_num_keys; i++)
        {
            if (!root->isleaf)
            {
                traverse_car_tree(root->children[i]);
            }
            printf("%s\n", root->car_data[i].car_VIN);
            printf("%s\n", root->car_data[i].car_name);
            printf("%s\n", root->car_data[i].car_type);
            printf("%s\n", root->car_data[i].car_colour);
            printf("%s\n", root->car_data[i].fuel_type);
            printf("%.2f\n", root->car_data[i].car_price);
            printf("%s\n", root->car_data[i].car_customer_ID);
            printf("%s\n", root->car_data[i].car_sales_person_ID);
            printf("%s\n",root->car_data[i].purchase_date);
            printf("\n");
        }
        if (!root->isleaf)
        {
            traverse_car_tree(root->children[root->car_num_keys]);
        }
    }
}

int car_exists(car_node *node, const char *VIN)
{
    int i = 0;
    while (i < node->car_num_keys && strcmp(VIN, node->car_data[i].car_VIN) > 0)
        i++;

    if (i < node->car_num_keys && strcmp(VIN, node->car_data[i].car_VIN) == 0)
        return 1; // Duplicate found

    if (node->isleaf)
        return 0;

    return car_exists(node->children[i], VIN);
}


status_code split_child_car(car_node *parent, int i, car_node *child)
{
    if (parent == NULL || child == NULL)
    {
        printf("Error: NULL pointer passed to split_child_car.\n");
        return FAILURE;
    }

    int mid = ORDER / 2;  // For ORDER = 5, mid = 2

    car_node *new_child = create_car_node(child->isleaf);
    if (new_child == NULL)
    {
        printf("Error: Memory allocation failed in split_child_car.\n");
        return FAILURE;
    }

    // Right child gets the keys after mid
    new_child->car_num_keys = child->car_num_keys - mid - 1;

    for (int j = 0; j < new_child->car_num_keys; j++)
    {
        new_child->car_data[j] = child->car_data[mid + 1 + j];
    }

    // If not leaf, move children as well
    if (!child->isleaf)
    {
        for (int j = 0; j <= new_child->car_num_keys; j++)
        {
            new_child->children[j] = child->children[mid + 1 + j];
        }
    }

    // Reduce number of keys in old child
    child->car_num_keys = mid;

    // Shift parent's children and data to make space
    for (int j = parent->car_num_keys; j >= i + 1; j--)
    {
        parent->children[j + 1] = parent->children[j];
    }
    parent->children[i + 1] = new_child;

    for (int j = parent->car_num_keys - 1; j >= i; j--)
    {
        parent->car_data[j + 1] = parent->car_data[j];
    }

    // Move the median key to the parent
    parent->car_data[i] = child->car_data[mid];
    parent->car_num_keys++;

    return SUCCESS;
}


status_code insert_non_full_car(car_node *node, car new_car)
{
    if (node == NULL)
    {
        printf("Error: NULL node passed to insert_non_full_car.\n");
        return FAILURE;
    }

    int i = node->car_num_keys - 1;

    if (node->isleaf)
    {
        while (i >= 0 && strcmp(new_car.car_VIN, node->car_data[i].car_VIN) < 0)
        {
            node->car_data[i + 1] = node->car_data[i];
            i--;
        }
        node->car_data[i + 1] = new_car;
        node->car_num_keys++;
        return SUCCESS;
    }
    else
    {
        while (i >= 0 && strcmp(new_car.car_VIN, node->car_data[i].car_VIN) < 0)
        {
            i--;
        }
        i++;

        if (node->children[i] == NULL)
        {
            printf("Error: NULL child node in insert_non_full_car.\n");
            return FAILURE;
        }

        if (node->children[i]->car_num_keys == ORDER - 1)
        {
            if (split_child_car(node, i, node->children[i]) == FAILURE)
            {
                return FAILURE;
            }

            if (strcmp(new_car.car_VIN, node->car_data[i].car_VIN) > 0)
            {
                i++;
            }
        }

        return insert_non_full_car(node->children[i], new_car);
    }
}


status_code insert_car(car_node **root, char *VIN, char *name, char *type,
    char *colour, char *fuel, float price,
    char *customer_ID, char *sales_person_ID,char* purchase_date)
{
    if (root == NULL || VIN == NULL || name == NULL || type == NULL || colour == NULL ||
        fuel == NULL || customer_ID == NULL || sales_person_ID == NULL)
    {
        
        return FAILURE;
    }

    if (*root != NULL && car_exists(*root, VIN))
    {
        printf("Duplicate VIN '%s' detected. Car not inserted.\n", VIN);
        return FAILURE;
    }

    car new_car;
    strcpy(new_car.car_VIN, VIN);
    strcpy(new_car.car_name, name);
    strcpy(new_car.car_type, type);
    strcpy(new_car.car_colour, colour);
    strcpy(new_car.fuel_type, fuel);
    new_car.car_price = price;
    strcpy(new_car.car_customer_ID, customer_ID);
    strcpy(new_car.car_sales_person_ID, sales_person_ID);
    strcpy(new_car.purchase_date,purchase_date);

    if (*root == NULL)
    {
        *root = create_car_node(1);
        if (*root == NULL)
        {
            printf("Error: Memory allocation failed in insert_car.\n");
            return FAILURE;
        }

        (*root)->car_data[0] = new_car;
        (*root)->car_num_keys = 1;
        return SUCCESS;
    }
    else
    {
        if ((*root)->car_num_keys == ORDER - 1)
        {
            car_node *new_root = create_car_node(0);
            if (new_root == NULL)
            {
                printf("Error: Memory allocation failed for new root in insert_car.\n");
                return FAILURE;
            }

            new_root->children[0] = *root;

            if (split_child_car(new_root, 0, *root) == FAILURE)
            {
                free(new_root);
                return FAILURE;
            }

            int i = 0;
            if (strcmp(new_car.car_VIN, new_root->car_data[0].car_VIN) > 0)
            {
                i = 1;
            }

            if (insert_non_full_car(new_root->children[i], new_car) == FAILURE)
            {
                free(new_root);
                return FAILURE;
            }

            *root = new_root;
        }
        else
        {
            if (insert_non_full_car(*root, new_car) == FAILURE)
            {
                return FAILURE;
            }
        }
    }

    return SUCCESS;
}



customer_node *create_customer_node(int is_leaf)
{
    customer_node *node = (customer_node *)malloc(sizeof(customer_node));
    node->isleaf = is_leaf;
    node->customer_num_keys = 0;
    for (int i = 0; i < ORDER; i++)
    {
        node->children[i] = NULL;
    }
    return node;
}

void traverse_customer_tree(customer_node *root)
{
    if (root != NULL)
    {
        for (int i = 0; i < root->customer_num_keys; i++)
        {
            if (!root->isleaf)
            {
                traverse_customer_tree(root->children[i]);
            }
            printf("%s\n", root->customer_data[i].customer_id);
            printf("%s\n", root->customer_data[i].customer_name);
            printf("%s\n", root->customer_data[i].mobile_no);
            printf("%s\n", root->customer_data[i].customer_address);
            printf("%s\n", root->customer_data[i].registration_number);
            printf("%s\n", root->customer_data[i].sold_car_VIN);
            printf("\n");
        }
        if (!root->isleaf)
        {
            traverse_customer_tree(root->children[root->customer_num_keys]);
        }
    }
}

status_code split_child_customer(customer_node *parent, int i, customer_node *child)
{
    if (!parent || !child)
    {
        printf("Error: NULL pointer encountered in split_child_customer.\n");
        return FAILURE;
    }

    int mid = ORDER / 2; // For ORDER = 5, mid = 2

    customer_node *new_child = create_customer_node(child->isleaf);
    if (!new_child)
    {
        printf("Error: Memory allocation failed in split_child_customer.\n");
        return FAILURE;
    }

    // Right child gets the keys after mid (i.e., [mid+1, ...])
    new_child->customer_num_keys = child->customer_num_keys - mid - 1;

    for (int j = 0; j < new_child->customer_num_keys; j++)
    {
        new_child->customer_data[j] = child->customer_data[mid + 1 + j];
    }

    // Move the child pointers if not a leaf
    if (!child->isleaf)
    {
        for (int j = 0; j <= new_child->customer_num_keys; j++)
        {
            new_child->children[j] = child->children[mid + 1 + j];
        }
    }

    // Reduce number of keys in original child
    child->customer_num_keys = mid;

    // Shift parent’s children to make space for new child
    for (int j = parent->customer_num_keys; j >= i + 1; j--)
    {
        parent->children[j + 1] = parent->children[j];
    }
    parent->children[i + 1] = new_child;

    // Shift parent's keys to make space for median key
    for (int j = parent->customer_num_keys - 1; j >= i; j--)
    {
        parent->customer_data[j + 1] = parent->customer_data[j];
    }

    // Move the median key from child to parent
    parent->customer_data[i] = child->customer_data[mid];
    parent->customer_num_keys++;

    return SUCCESS;
}


status_code insert_non_full_customer(customer_node *node, customer new_customer)
{
    if (!node)
    {
        printf("Error: NULL node encountered in insert_non_full_customer.\n");
        return FAILURE;
    }

    int i = node->customer_num_keys - 1;

    if (node->isleaf)
    {
        while (i >= 0 && strcmp(new_customer.sold_car_VIN, node->customer_data[i].sold_car_VIN) < 0)
        {
            node->customer_data[i + 1] = node->customer_data[i];
            i--;
        }
        node->customer_data[i + 1] = new_customer;
        node->customer_num_keys++;
        return SUCCESS;
    }
    else
    {
        while (i >= 0 && strcmp(new_customer.sold_car_VIN, node->customer_data[i].sold_car_VIN) < 0)
        {
            i--;
        }
        i++;

        if (node->children[i]->customer_num_keys == ORDER - 1)
        {
            if (split_child_customer(node, i, node->children[i]) == FAILURE)
            {
                printf("Error: Failed to split child during insert_non_full_customer.\n");
                return FAILURE;
            }

            if (strcmp(new_customer.sold_car_VIN, node->customer_data[i].sold_car_VIN) > 0)
            {
                i++;
            }
        }

        return insert_non_full_customer(node->children[i], new_customer);
    }
}

status_code insert_customer(customer_node **root, char *customer_id, char *name, char *mobile,
                            char *address, char *reg_no, char *sold_car_VIN)
{
    if (!root || !customer_id || !name || !mobile || !address || !reg_no || !sold_car_VIN)
    {
        printf("Error: NULL argument passed to insert_customer.\n");
        return FAILURE;
    }

    customer new_customer;
    strcpy(new_customer.customer_id, customer_id);
    strcpy(new_customer.customer_name, name);
    strcpy(new_customer.mobile_no, mobile);
    strcpy(new_customer.customer_address, address);
    strcpy(new_customer.registration_number, reg_no);
    strcpy(new_customer.sold_car_VIN, sold_car_VIN);

    if (*root == NULL)
    {
        *root = create_customer_node(1);
        if (!*root)
        {
            printf("Error: Memory allocation failed for root in insert_customer.\n");
            return FAILURE;
        }
        (*root)->customer_data[0] = new_customer;
        (*root)->customer_num_keys = 1;
        return SUCCESS;
    }

    if ((*root)->customer_num_keys == ORDER - 1)
    {
        customer_node *new_root = create_customer_node(0);
        if (!new_root)
        {
            printf("Error: Memory allocation failed for new_root in insert_customer.\n");
            return FAILURE;
        }

        new_root->children[0] = *root;

        if (split_child_customer(new_root, 0, *root) == FAILURE)
        {
            printf("Error: Failed to split full root in insert_customer.\n");
            return FAILURE;
        }

        int i = (strcmp(new_customer.sold_car_VIN, new_root->customer_data[0].sold_car_VIN) > 0) ? 1 : 0;
        status_code result = insert_non_full_customer(new_root->children[i], new_customer);
        if (result == FAILURE)
        {
            printf("Error: Insertion failed after root split in insert_customer.\n");
            return FAILURE;
        }

        *root = new_root;
    }
    else
    {
        return insert_non_full_customer(*root, new_customer);
    }

    return SUCCESS;
}



sold_car_node *create_sold_car_node(boolean is_leaf)
{
    sold_car_node *node = (sold_car_node *)malloc(sizeof(sold_car_node));
    node->isleaf = is_leaf;
    node->sold_car_num_keys = 0;
    for (int i = 0; i < ORDER; i++)
    {
        node->children[i] = NULL;
    }
    return node;
}

void traverse_sold_car_tree(sold_car_node *root)
{
    if (root != NULL)
    {
        for (int i = 0; i < root->sold_car_num_keys; i++)
        {
            if (!root->isleaf)
            {
                traverse_sold_car_tree(root->children[i]);
            }
            printf("%s %s %d %s\n", root->sold_car[i].VIN,
                   root->sold_car[i].Payment_Type, root->sold_car[i].loan_period, root->sold_car[i].purchase_date);
        }
        if (!root->isleaf)
        {
            traverse_sold_car_tree(root->children[root->sold_car_num_keys]);
        }
    }
}

status_code split_child_sold_car(sold_car_node *parent, int i, sold_car_node *child) {
    if (!parent || !child)
    {
        printf("Error: NULL pointer encountered in split_child_sold_car.\n");
        return FAILURE;
    }

    int mid = ORDER / 2;
    
    sold_car_node *new_child = create_sold_car_node(child->isleaf);

    if (new_child == NULL) {
        printf("Error: Memory allocation failed while splitting sold car node.\n");
        return FAILURE;
    }

    new_child->sold_car_num_keys = child->sold_car_num_keys - mid  - 1;

    for (int j = 0; j < new_child->sold_car_num_keys ; j++) {
        new_child->sold_car[j] = child->sold_car[j + mid+1];
    }

    if (!child->isleaf) {
        for (int j = 0; j <= new_child->sold_car_num_keys; j++) {
            new_child->children[j] = child->children[j + mid + 1];
        }
    }

    child->sold_car_num_keys = mid;

    for (int j = parent->sold_car_num_keys; j >= i + 1; j--) {
        parent->children[j + 1] = parent->children[j];
    }
    parent->children[i + 1] = new_child;

    for (int j = parent->sold_car_num_keys - 1; j >= i; j--) {
        parent->sold_car[j + 1] = parent->sold_car[j];
    }

    parent->sold_car[i] = child->sold_car[mid];
    parent->sold_car_num_keys++;

    return SUCCESS;
}

status_code insert_non_full_sold_car(sold_car_node *node, sold_car_info new_car) {
    int i = node->sold_car_num_keys - 1;

    if (node->isleaf) {
        while (i >= 0 && (new_car.loan_period < node->sold_car[i].loan_period ||
                          (new_car.loan_period == node->sold_car[i].loan_period &&
                           strcmp(new_car.VIN, node->sold_car[i].VIN) < 0))) {
            node->sold_car[i + 1] = node->sold_car[i];
            i--;
        }
        node->sold_car[i + 1] = new_car;
        node->sold_car_num_keys++;
        return SUCCESS;
    } else {
        while (i >= 0 && (new_car.loan_period < node->sold_car[i].loan_period ||
                          (new_car.loan_period == node->sold_car[i].loan_period &&
                           strcmp(new_car.VIN, node->sold_car[i].VIN) < 0))) {
            i--;
        }
        i++;

        if (node->children[i]->sold_car_num_keys == ORDER - 1) {
            if (split_child_sold_car(node, i, node->children[i]) == FAILURE) {
                return FAILURE;
            }

            if (new_car.loan_period > node->sold_car[i].loan_period ||
                (new_car.loan_period == node->sold_car[i].loan_period &&
                 strcmp(new_car.VIN, node->sold_car[i].VIN) > 0)) {
                i++;
            }
        }
        return insert_non_full_sold_car(node->children[i], new_car);
    }
}

status_code insert_sold_car(sold_car_node **root, char *VIN, char *Payment_Type, int loan_period, char *purchase_date) {
    sold_car_info new_car;
    strcpy(new_car.VIN, VIN);
    strcpy(new_car.Payment_Type, Payment_Type);
    new_car.loan_period = loan_period;
    strcpy(new_car.purchase_date, purchase_date);

    if (*root == NULL) {
        *root = create_sold_car_node(TRUE);
        if (*root == NULL) {
            printf("Error: Memory allocation failed while creating root sold car node.\n");
            return FAILURE;
        }
        (*root)->sold_car[0] = new_car;
        (*root)->sold_car_num_keys = 1;
        return SUCCESS;
    }

    if ((*root)->sold_car_num_keys == ORDER - 1) {
        sold_car_node *new_root = create_sold_car_node(FALSE);
        if (new_root == NULL) {
            printf("Error: Memory allocation failed while creating new root sold car node.\n");
            return FAILURE;
        }

        new_root->children[0] = *root;

        if (split_child_sold_car(new_root, 0, *root) == FAILURE) {
            return FAILURE;
        }

        int i = 0;
        if (new_car.loan_period > new_root->sold_car[0].loan_period ||
            (new_car.loan_period == new_root->sold_car[0].loan_period &&
             strcmp(new_car.VIN, new_root->sold_car[0].VIN) > 0)) {
            i = 1;
        }

        if (insert_non_full_sold_car(new_root->children[i], new_car) == FAILURE) {
            return FAILURE;
        }

        *root = new_root;
    } else {
        if (insert_non_full_sold_car(*root, new_car) == FAILURE) {
            return FAILURE;
        }
    }

    return SUCCESS;
}




sales_person_node *create_salesperson_node(int is_leaf)
{
    sales_person_node *new_node = (sales_person_node *)malloc(sizeof(sales_person_node));
    new_node->sales_person_num_keys = 0;
    new_node->isleaf = is_leaf;
    for (int i = 0; i < ORDER; i++)
    {
        new_node->children[i] = NULL;
    }
    return new_node;
}

void traverse_salesperson_tree(sales_person_node *root)
{
    if (root == NULL)
        return;

        for (int i = 0; i < root->sales_person_num_keys; i++) {
            if (!root->isleaf) {
                traverse_salesperson_tree(root->children[i]);
            }
        
            sales_person sp = root->sales_person_data[i];
            printf("\nSalesperson ID: %s\n", sp.sales_person_ID);
            printf("Salesperson Name: %s\n", sp.sales_person_name);
            printf("Sales Target: %.2f\n", sp.sales_target);
            printf("Sales Achieved: %.2f\n", sp.sales_achived);
            printf("Commission per Sale: %.2f\n", sp.commission_per_sale);
            printf("\n");
        
            if (sp.customer_tree_root != NULL) {
                if (sp.customer_tree_root->customer_children != NULL) {
                    printf("Customers ---:\n");
                    traverse_customer_tree(sp.customer_tree_root->customer_children);
                } else {
                    printf("Customer tree exists but no customers inserted yet.\n");
                }
            } else {
                printf("Customer tree not initialized.\n");
            }
        
            if (sp.sold_car_root != NULL) {
                printf("\nSold Cars:\n");
                traverse_sold_car_tree(sp.sold_car_root);
            } else {
                printf("\nSold car tree not initialized.\n");
            }
        
            printf("--------------------------------------------------\n");
        }
        
        if (!root->isleaf) {
            traverse_salesperson_tree(root->children[root->sales_person_num_keys]);
        }
        
}




sales_person *search_salesperson_by_id(sales_person_node *root, const char *id)
{
    if (root == NULL)
        return NULL;

    for (int i = 0; i < root->sales_person_num_keys; i++) {
        if (strcmp(root->sales_person_data[i].sales_person_ID, id) == 0) {
            return &(root->sales_person_data[i]);
        }
    }

    if (root->isleaf)
        return NULL;

    for (int i = 0; i <= root->sales_person_num_keys; i++) {
        sales_person *found = search_salesperson_by_id(root->children[i], id);
        if (found)
            return found;
    }

    return NULL;
}


status_code split_child_salesperson(sales_person_node *parent, int i, sales_person_node *child) {
    if (!parent || !child) {
        printf("Error: NULL pointer encountered in split_child_salesperson.\n");
        return FAILURE;
    }

    // Create new node (will become the right sibling)
    sales_person_node *new_child = create_salesperson_node(child->isleaf);
    if (!new_child) {
        printf("Memory allocation failed for new child in split.\n");
        return FAILURE;
    }

    // For ORDER=5, we split at index 2 (0-based)
    int split_pos = (ORDER ) / 2;  // This gives 2 for ORDER=5
    new_child->sales_person_num_keys = (ORDER - 1) - (split_pos + 1);  // 2 keys in new node

    // Copy the upper half of child to new_child
    for (int j = 0; j < new_child->sales_person_num_keys; j++) {
        new_child->sales_person_data[j] = child->sales_person_data[split_pos + 1 + j];
    }

    // If not leaf, copy the children pointers
    if (!child->isleaf) {
        for (int j = 0; j <= new_child->sales_person_num_keys; j++) {
            new_child->children[j] = child->children[split_pos + 1 + j];
        }
    }

    // Reduce the number of keys in original child
    child->sales_person_num_keys = split_pos;  // Now has 2 keys

    // Make space in parent for new child
    for (int j = parent->sales_person_num_keys; j > i; j--) {
        parent->children[j + 1] = parent->children[j];
    }
    parent->children[i + 1] = new_child;

    // Make space for the promoted key
    for (int j = parent->sales_person_num_keys - 1; j >= i; j--) {
        parent->sales_person_data[j + 1] = parent->sales_person_data[j];
    }

    // Promote the middle key from child to parent
    parent->sales_person_data[i] = child->sales_person_data[split_pos];
    parent->sales_person_num_keys++;

    return SUCCESS;
}

status_code insert_non_full_salesperson(sales_person_node *node, sales_person new_salesperson) {
    int i = node->sales_person_num_keys - 1;

    if (node->isleaf) {
        // Find the correct position for the new key
        while (i >= 0 && strcmp(new_salesperson.sales_person_ID, node->sales_person_data[i].sales_person_ID) < 0) {
            node->sales_person_data[i + 1] = node->sales_person_data[i];
            i--;
        }
        // Insert new key
        node->sales_person_data[i + 1] = new_salesperson;
        node->sales_person_num_keys++;
        return SUCCESS;
    } else {
        // Find the child to descend into
        while (i >= 0 && strcmp(new_salesperson.sales_person_ID, node->sales_person_data[i].sales_person_ID) < 0) {
            i--;
        }
        i++;

        // Check if child is full
        if (node->children[i]->sales_person_num_keys == ORDER - 1) {
            if (split_child_salesperson(node, i, node->children[i]) == FAILURE) {
                return FAILURE;
            }
            // After split, decide which child to go into
            if (strcmp(new_salesperson.sales_person_ID, node->sales_person_data[i].sales_person_ID) > 0) {
                i++;
            }
        }
        return insert_non_full_salesperson(node->children[i], new_salesperson);
    }
}

status_code insert_salesperson(sales_person_node **root, char *sales_person_ID, char *sales_person_name, float sales_target, float sales_achieved,
    char *customer_id, char *name, char *mobile, char *address, char *reg_no, char *sold_car_VIN, char *payment_type, int loan_period,
    char *purchase_date)
{
    sales_person *existing = search_salesperson_by_id(*root, sales_person_ID);
    if (existing != NULL) {
        if (insert_customer(&(existing->customer_tree_root->customer_children), customer_id, name, mobile, address, reg_no, sold_car_VIN) == FAILURE ||
            insert_sold_car(&(existing->sold_car_root), sold_car_VIN, payment_type, loan_period, purchase_date) == FAILURE) {
            printf("Failed to insert customer or sold car for existing salesperson.\n");
            return FAILURE;
        }
        return SUCCESS;
    }

    sales_person new_sales_person;
    strcpy(new_sales_person.sales_person_ID, sales_person_ID);
    strcpy(new_sales_person.sales_person_name, sales_person_name);
    new_sales_person.sales_target = sales_target;
    new_sales_person.sales_achived = sales_achieved;
    new_sales_person.commission_per_sale = 0.02 * sales_achieved;

    new_sales_person.customer_tree_root = (customer_root *)malloc(sizeof(customer_root));
    if (!new_sales_person.customer_tree_root) {
        printf("Memory allocation failed for customer tree.\n");
        return FAILURE;
    }
    new_sales_person.customer_tree_root->customer_children = NULL;
    strcpy(new_sales_person.customer_tree_root->sales_person_id, "");

    new_sales_person.sold_car_root = NULL;

    if (insert_customer(&(new_sales_person.customer_tree_root->customer_children), customer_id, name, mobile, address, reg_no, sold_car_VIN) == FAILURE ||
        insert_sold_car(&(new_sales_person.sold_car_root), sold_car_VIN, payment_type, loan_period, purchase_date) == FAILURE) {
        printf("Failed to insert customer or sold car for new salesperson.\n");
        return FAILURE;
    }

    if (*root == NULL) {
        *root = create_salesperson_node(1);
        if (!*root) {
            printf("Memory allocation failed for root.\n");
            return FAILURE;
        }
        (*root)->sales_person_data[0] = new_sales_person;
        (*root)->sales_person_num_keys = 1;
        return SUCCESS;
    }

    if ((*root)->sales_person_num_keys == ORDER - 1) {
        sales_person_node *new_root = create_salesperson_node(0);
        if (!new_root) {
            printf("Memory allocation failed for new root.\n");
            return FAILURE;
        }
        new_root->children[0] = *root;

        if (split_child_salesperson(new_root, 0, *root) == FAILURE) {
            return FAILURE;
        }

        int i = (strcmp(new_sales_person.sales_person_ID,new_root->sales_person_data[0].sales_person_ID)<0) ? 0 : 1;

        if (insert_non_full_salesperson(new_root->children[i], new_sales_person) == FAILURE) {
            return FAILURE;
        }

        *root = new_root;
    } else {
        if (insert_non_full_salesperson(*root, new_sales_person) == FAILURE) {
            return FAILURE;
        }
    }

    return SUCCESS;
}


void creating_showroom(const char *filename, showroom *showrooms)
{
    showrooms->sold_cars_root = (car_root *)malloc(sizeof(car_root));
    showrooms->avialable_cars_root = (car_root *)malloc(sizeof(car_root));
    showrooms->salespersons_root = (sales_person_root *)malloc(sizeof(sales_person_root));

    if (!showrooms->sold_cars_root || !showrooms->avialable_cars_root || !showrooms->salespersons_root)
    {
        printf("Memory allocation failed\n");
        return;
    }

    showrooms->sold_cars_root->car_children = NULL;
    showrooms->avialable_cars_root->car_children = NULL;
    showrooms->salespersons_root->sales_person_children = NULL;
    showrooms->available_cars = 0;
    showrooms->sold_cars = 0;

    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Error opening the file\n");
        return;
    }

    char buffer[SIZE];
    int line_counter = 0;
    int is_salesperson_section = 0;

    while (fgets(buffer, sizeof(buffer), file))
    {
        buffer[strcspn(buffer, "\n")] = '\0'; // Remove trailing newline

        // Store the first two lines in appropriate fields
        if (line_counter == 0)
        {
            strcpy(showrooms->showroom_id,buffer);
            strcpy(showrooms->salespersons_root->showroom_id, buffer);
            line_counter++;
            continue;
        }
        else if (line_counter == 1)
        {
            strcpy(showrooms->sold_cars_root->unique_id, buffer);
            strcpy(showrooms->avialable_cars_root->unique_id, buffer);
            line_counter++;
            continue;
        }

        if (strncmp(buffer, "SalesPerson", 11) == 0)
        {
            is_salesperson_section = 1;
            continue;
        }

        if (!is_salesperson_section)
        {
            char car_VIN[SIZE], car_name[SIZE], car_type[SIZE], car_colour[SIZE], fuel_type[SIZE];
            float car_price;
            char car_customer_ID[SIZE], car_sales_person_ID[SIZE],purchase_date[SIZE];

            sscanf(buffer, "%[^,],%[^,],%[^,],%[^,],%[^,],%f,%[^,],%[^,],%[^,]",
                   car_name, car_VIN, car_type, car_colour, fuel_type, &car_price, car_customer_ID, car_sales_person_ID,purchase_date);

            if (strcmp(car_customer_ID, "NULL") == 0)
            {
                showrooms->available_cars++;
                insert_car(&(showrooms->avialable_cars_root->car_children), car_VIN, car_name, car_type,
                           car_colour, fuel_type, car_price, car_customer_ID, car_sales_person_ID,purchase_date);
            }
            else
            {
                showrooms->sold_cars++;
                insert_car(&(showrooms->sold_cars_root->car_children), car_VIN, car_name, car_type,
                           car_colour, fuel_type, car_price, car_customer_ID, car_sales_person_ID,purchase_date);
            }
        }
        else
        {
            char sales_person_ID[SIZE], sales_person_name[SIZE], customer_id[SIZE], customer_name[SIZE];
            char mobile_no[20], customer_address[SIZE], registration_number[SIZE], sold_car_VIN[SIZE];
            char Payment_Type[SIZE], purchase_date[SIZE];
            float sales_target, sales_achived;
            int loan_period;

            sscanf(buffer, "%[^,],%[^,],%f,%f,%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%d,%[^,]",
                   sales_person_ID, sales_person_name, &sales_target, &sales_achived,
                   customer_id, customer_name, mobile_no, customer_address, registration_number,
                   sold_car_VIN, Payment_Type, &loan_period, purchase_date);

            insert_salesperson(&(showrooms->salespersons_root->sales_person_children), sales_person_ID, sales_person_name, sales_target,sales_achived, 
            customer_id, customer_name, mobile_no, customer_address,registration_number, sold_car_VIN, Payment_Type, loan_period, purchase_date);
        }
    }

    fclose(file);
}



void traverse_and_insert_cars(car_node *node, car_node **dest, int *count) 
{
    if (node == NULL) return;

    if (*dest == NULL) {
        *dest = create_car_node(1);
    }


    int i;
    for (i = 0; i < node->car_num_keys; i++) {
        // Recur for child before key
        if (!node->isleaf && node->children[i] != NULL) {
        traverse_and_insert_cars(node->children[i], dest, count); 
        }

        // Insert the key
        insert_car(dest, node->car_data[i].car_VIN, node->car_data[i].car_name,
                   node->car_data[i].car_type, node->car_data[i].car_colour,
                   node->car_data[i].fuel_type, node->car_data[i].car_price,
                   node->car_data[i].car_customer_ID, node->car_data[i].car_sales_person_ID,node->car_data[i].purchase_date);
        (*count)++;
    }

    // Recur for last child
    if (!node->isleaf && node->children[i] != NULL){
    traverse_and_insert_cars(node->children[i], dest, count);
    }

}




void insert_existing_salesperson(sales_person_node **root, sales_person *sp)
{
    

    // If root is NULL, create an empty node but do NOT insert directly
    if (*root == NULL)
    {
        *root = create_salesperson_node(1); // create empty leaf
    }

    // If root is full, split it
    if ((*root)->sales_person_num_keys == ORDER - 1)
    {
        sales_person_node *new_root = create_salesperson_node(0); // new root is internal
        new_root->children[0] = *root;

        if (split_child_salesperson(new_root, 0, *root) == FAILURE)
        {
            printf("Error: Failed to split child during insertion.\n");
            return;
        }

        // Choose correct child based on ordering
        int i = 0;
        if (sp->sales_achived > new_root->sales_person_data[0].sales_achived ||
            (sp->sales_achived == new_root->sales_person_data[0].sales_achived &&
             strcmp(sp->sales_person_ID, new_root->sales_person_data[0].sales_person_ID) > 0))
        {
            i = 1;
        }

        if (insert_non_full_salesperson(new_root->children[i], *sp) == FAILURE)
        {
            printf("Error: Failed to insert salesperson.\n");
        }

        *root = new_root; // update root
    }
    else
    {
        if (insert_non_full_salesperson(*root, *sp) == FAILURE)
        {
            printf("Error: Failed to insert salesperson into non-full root.\n");
        }
    }
}


void merge_salesperson_tree(sales_person_node *src_root, sales_person_node **dest_root)
{
    
    if (src_root == NULL)
        return;

    // Traverse all children and data
    for (int i = 0; i < src_root->sales_person_num_keys; i++)
    {
        // Recursively merge left child
        if (!src_root->isleaf)
            merge_salesperson_tree(src_root->children[i], dest_root);

        // Insert salesperson only if not already present
        if (search_salesperson_by_id(*dest_root, src_root->sales_person_data[i].sales_person_ID) == NULL)
        {
            insert_existing_salesperson(dest_root, &src_root->sales_person_data[i]);
        }
    }

    // Merge rightmost child
    if (!src_root->isleaf)
        merge_salesperson_tree(src_root->children[src_root->sales_person_num_keys], dest_root);
}



showroom merge_showrooms(showroom showrooms[], int count) {
    showroom merged;

    // Allocate memory for merged roots
    merged.sold_cars_root = (car_root *)malloc(sizeof(car_root));
    merged.avialable_cars_root = (car_root *)malloc(sizeof(car_root));
    merged.salespersons_root = (sales_person_root *)malloc(sizeof(sales_person_root));

    // Initialize root pointers
    merged.sold_cars_root->car_children = NULL;
    merged.avialable_cars_root->car_children = NULL;
    merged.salespersons_root->sales_person_children = NULL;

    // Copy showroom ID and unique IDs from first showroom (optional logic, up to you)
    strcpy(merged.sold_cars_root->unique_id, "");
    strcpy(merged.avialable_cars_root->unique_id, "");
    strcpy(merged.salespersons_root->showroom_id, "");

    merged.available_cars = 0;
    merged.sold_cars = 0;

    for (int i = 0; i < count; i++) {
        // Merge Sold Cars
        traverse_and_insert_cars(showrooms[i].sold_cars_root->car_children,
            &merged.sold_cars_root->car_children,&merged.sold_cars);

        // Merge Available Cars
        traverse_and_insert_cars(showrooms[i].avialable_cars_root->car_children,
             &merged.avialable_cars_root->car_children, &merged.available_cars);

        // Merge Salespersons (No duplicates expected)
        merge_salesperson_tree(showrooms[i].salespersons_root->sales_person_children,
                               &merged.salespersons_root->sales_person_children);
    }

    return merged;
}




typedef struct hash_entry {
    char model_name[SIZE];  // Car model name
    int count;              // Number of times the car has been sold
    struct hash_entry *next;  // For collision resolution (chaining)
} hash_entry;

hash_entry *hash_table[HASH_SIZE];

unsigned int hash_function(const char *str) {
    unsigned int hash = 0;
    while (*str) {
        hash = (hash * 31) + *str++;
    }
    return hash % HASH_SIZE;
}

void update_car_count(const char *model_name) {
    if (!model_name) {
        fprintf(stderr, "Error: NULL model name passed to update_car_count\n");
        return;
    }

    unsigned int index = hash_function(model_name);
    hash_entry *curr = hash_table[index];

    // Check if model already exists
    while (curr) {
        if (strcmp(curr->model_name, model_name) == 0) {
            curr->count++;
            return;
        }
        curr = curr->next;
    }

    // Not found, insert new
    hash_entry *new_entry = (hash_entry *)malloc(sizeof(hash_entry));
    if (!new_entry) {
        fprintf(stderr, "Memory allocation failed for hash entry\n");
        return;
    }

    strcpy(new_entry->model_name, model_name);
    new_entry->count = 1;
    new_entry->next = hash_table[index];
    hash_table[index] = new_entry;
}


void traverse_sold_cars(car_node *root) {
    if (!root) return;

    for (int i = 0; i < root->car_num_keys; i++) {
        if (strlen(root->car_data[i].car_name) > 0) {
            update_car_count(root->car_data[i].car_name);
        } else {
            fprintf(stderr, "Warning: Empty car name encountered\n");
        }
    }

    // Traverse children
    if (!root->isleaf) {
        for (int i = 0; i <= root->car_num_keys; i++) {
            traverse_sold_cars(root->children[i]);
        }
    }
}


void find_most_popular_car_in_showroom(car_node *sold_cars_root) {
    if (!sold_cars_root) {
        printf("Showroom has no sold cars.\n");
        return;
    }

    // Initialize hash table
    for (int i = 0; i < HASH_SIZE; i++) {
        hash_table[i] = NULL;
    }

    // Step 1: Traverse and build hash map
    traverse_sold_cars(sold_cars_root);

    // Step 2: Find max
    int max_count = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        hash_entry *curr = hash_table[i];
        while (curr) {
            if (curr->count > max_count) {
                max_count = curr->count;
            }
            curr = curr->next;
        }
    }

    // Step 3: Print all cars with max count
    printf("Most Popular Cars (Sold %d times):\n", max_count);
    for (int i = 0; i < HASH_SIZE; i++) {
        hash_entry *curr = hash_table[i];
        while (curr) {
            if (curr->count == max_count) {
                printf(" - %s\n", curr->model_name);
            }
            curr = curr->next;
        }
    }

    // Step 4: Free memory
    for (int i = 0; i < HASH_SIZE; i++) {
        hash_entry *curr = hash_table[i];
        while (curr) {
            hash_entry *temp = curr;
            curr = curr->next;
            free(temp);
        }
    }
}


// Function to extract year and month from date string
int extract_year_month(const char *date_str, int *year, int *month) {
    int result = sscanf(date_str, "%d-%d", year, month);
    if (result != 2) {
        fprintf(stderr, "Error: Invalid date format in %s. Expected format: YYYY-MM-DD.\n", date_str);
        return -1; // Invalid date format
    }
    return 0;
}

// Check if the car was sold in the previous month
int is_previous_month(const char *purchase_date) {
    // Get the current date
    time_t t = time(NULL);
    struct tm current_time = *localtime(&t);

    int sold_year, sold_month;
    if (extract_year_month(purchase_date, &sold_year, &sold_month) == -1) {
        return 0;  // Return 0 (false) if date format is invalid
    }


    // Check if the car's purchase date is in the previous month
    if (sold_year == current_time.tm_year + 1900) { // tm_year is years since 1900, adjust for current yea
        // If the month is previous to the current month
        if (sold_month == current_time.tm_mon) {  // sold_month is 1-indexed, tm_mon is 0-indexed
            return 1; // It's in the previous month
        }
    } else if (sold_year == current_time.tm_year + 1900 - 1) {
        // If the car was sold in December of the last year and the current month is January
        if (sold_month == 12 && current_time.tm_mon == 0) {
            return 1; // It's in the previous month (December -> January)
        }
    }
    return 0; // Not in the previous month
}


void ensure_car_in_hash_table(char *car_name) {
    int index = hash_function(car_name);
    hash_entry *curr = hash_table[index];

    // Traverse the linked list at this index to check if car is already in the hash table
    while (curr) {
        if (strcmp(curr->model_name, car_name) == 0) {
            return; // If car already exists, do nothing
        }
        curr = curr->next;
    }

    // If car model is not found, add it with a sales count of 0
    hash_entry *new_entry = malloc(sizeof(hash_entry));
    if (new_entry) {
        strcpy(new_entry->model_name, car_name);
        new_entry->count = 0;  // Initialize sales count to 0
        new_entry->next = hash_table[index];  // Insert at the front of the linked list
        hash_table[index] = new_entry;
    }
}


void traverse_sold_cars_and_update(car_node *root) {
    if (!root) {
        fprintf(stderr, "Error: NULL root in B-tree traversal.\n");
        return; // Return if the root is NULL
    }

    // Traverse the sold cars and update the hash table
    for (int i = 0; i < root->car_num_keys; i++) {
        if (strlen(root->car_data[i].car_name) > 0) {
            // If the car was sold in the previous month, increment its sales count
            if (is_previous_month(root->car_data[i].purchase_date)) {
                update_car_count(root->car_data[i].car_name); // Increment sales count
            } else {
                // If no sales in previous month, ensure it's added with 0 sales
                ensure_car_in_hash_table(root->car_data[i].car_name);
            }
        }
    }

    // Recursively traverse children if not a leaf
    if (!root->isleaf) {
        for (int i = 0; i <= root->car_num_keys; i++) {
            traverse_sold_cars_and_update(root->children[i]);
        }
    }
}


// Ensures car is added to the hash table with 0 sales if not already present
void predict_next_month_sales(car_node *sold_cars_root) {


    // Step 1: Validate car tree root (B-tree)
    if (!sold_cars_root) {
        fprintf(stderr, "Error: Sold cars root is NULL.\n");
        return; // Return if the root is NULL
    }

    // Step 2: Initialize the hash table
    for (int i = 0; i < HASH_SIZE; i++) {
        hash_table[i] = NULL;
    }

    // Step 3: Traverse the B-tree and update the sales count for cars sold in the previous month
    traverse_sold_cars_and_update(sold_cars_root);

    // Step 4: Predict next month's sales based on the updated sales count
    printf("Predicted Sales for Next Month:\n");

    int found_sales = 0;  // To track if we found any car sales data in the hash table

    for (int i = 0; i < HASH_SIZE; i++) {
        hash_entry *curr = hash_table[i];
        while (curr) {
            // If sales were made (count > 0)
            if (curr->count > 0) {
                found_sales = 1;
                int predicted_sales = curr->count * (1 + GROWTH_RATE);
                printf("Car Model: %s, Predicted Sales: %d (from previous month sales)\n", curr->model_name, predicted_sales);
            } else {
                // If no sales in the previous month (count == 0)
                found_sales = 1;
                printf("Car Model: %s, Predicted Sales: %d (no sales last month)\n", curr->model_name, DEFAULT_SALES);
            }
            curr = curr->next;
        }
    }

    if (!found_sales) {
        printf("No car sales data found.\n");
    }

    // Step 5: Cleanup - Free memory for the hash table entries
    for (int i = 0; i < HASH_SIZE; i++) {
        hash_entry *entry = hash_table[i];
        while (entry) {
            hash_entry *temp = entry;
            entry = entry->next;
            free(temp); // Free each hash entry
        }
        hash_table[i] = NULL; // Set the table entry to NULL after freeing
    }
}

void traverse_sold_cars_and_print(sold_car_node *sold_car_root, customer_node *customer_root) {
    if (!sold_car_root || !customer_root) {
        fprintf(stderr, "Error: Null root in B-tree traversal (sold car tree or customer tree missing).\n");
        return;
    }

    int customer_found = 0;  // To track if any customer is found

    for (int i = 0; i < sold_car_root->sold_car_num_keys; i++) {
        sold_car_info car = sold_car_root->sold_car[i];

        // Check if the car has an EMI plan within the specified range
        if (car.loan_period >= 36 && car.loan_period <= 48) {
            int customer_match = 0;  // Flag to check if a matching customer is found

            // Find the corresponding customer based on sold_car_VIN
            for (int j = 0; j < customer_root->customer_num_keys; j++) {
                customer cust = customer_root->customer_data[j];
                if (strcmp(cust.sold_car_VIN, car.VIN) == 0) {
                    // Print the customer details who has the EMI plan in the desired range
                    printf("Customer Name: %s\n", cust.customer_name);
                    printf("Customer ID: %s\n", cust.customer_id);
                    printf("Customer Mobile: %s\n", cust.mobile_no);
                    printf("EMI Loan Period: %d months\n", car.loan_period);
                    printf("Address: %s\n", cust.customer_address);
                    printf("-----------------------------------\n");
                    customer_match = 1;
                    customer_found = 1;
                    break;
                }
            }

            if (!customer_match) {
                fprintf(stderr, "Error: No matching customer found for sold car with VIN: %s\n", car.VIN);
            }
        }
    }

    if (!customer_found) {
        printf("No customers found with EMI plans between 36 and 48 months.\n");
    }

    // Recursively traverse the children nodes if not a leaf node
    if (!sold_car_root->isleaf) {
        for (int i = 0; i <= sold_car_root->sold_car_num_keys; i++) {
            traverse_sold_cars_and_print(sold_car_root->children[i], customer_root);
        }
    }
}

void traverse_salesperson_and_print(sales_person_node *salesperson_root) {
    if (!salesperson_root) {
        fprintf(stderr, "Error: Null root in salesperson B-tree traversal.\n");
        return;
    }

    int salesperson_found = 0;  // To track if any salesperson is found

    // Traverse through the salesperson tree
    for (int i = 0; i < salesperson_root->sales_person_num_keys; i++) {
        sales_person salesperson = salesperson_root->sales_person_data[i];

        // Ensure the salesperson has both a sold car B-tree and a customer B-tree
        if (salesperson.sold_car_root && salesperson.customer_tree_root->customer_children) {
            salesperson_found = 1;
            // Call the function to print customers with EMI plans between 36 and 48 months
            traverse_sold_cars_and_print(salesperson.sold_car_root, salesperson.customer_tree_root->customer_children);
        }
    }

    if (!salesperson_found) {
        fprintf(stderr, "Error: No salesperson found in the tree.\n");
    }

    // Recursively traverse children if not a leaf node
    if (!salesperson_root->isleaf) {
        for (int i = 0; i <= salesperson_root->sales_person_num_keys; i++) {
            traverse_salesperson_and_print(salesperson_root->children[i]);
        }
    }
}


car get_predecessor(car_node* node) {
    while (!node->isleaf) {
        node = node->children[node->car_num_keys];
    }
    return node->car_data[node->car_num_keys - 1];
}

// Get the Successor (Leftmost Car in Right Subtree)
car get_successor(car_node* node) {
    while (!node->isleaf) {
        node = node->children[0];
    }
    return node->car_data[0];
}

// Merge Two Nodes
void merge_nodes(car_node* parent, int index) {
    car_node* left = parent->children[index];
    car_node* right = parent->children[index + 1];

    // Move key from parent to left
    left->car_data[left->car_num_keys] = parent->car_data[index];

    // Move keys from right node to left
    for (int i = 0; i < right->car_num_keys; i++) {
        left->car_data[left->car_num_keys + 1 + i] = right->car_data[i];
    }

    // Move children if not leaf
    if (!left->isleaf) {
        for (int i = 0; i <= right->car_num_keys; i++) {
            left->children[left->car_num_keys + 1 + i] = right->children[i];
        }
    }

    // Shift parent keys and children
    for (int i = index; i < parent->car_num_keys - 1; i++) {
        parent->car_data[i] = parent->car_data[i + 1];
        parent->children[i + 1] = parent->children[i + 2];
    }

    left->car_num_keys += right->car_num_keys + 1;
    parent->car_num_keys--;

    free(right);
}

// Borrow from Left Sibling
void borrow_from_left(car_node* parent, int index) {
    car_node* left = parent->children[index - 1];
    car_node* current = parent->children[index];

    // Shift elements in current node
    for (int i = current->car_num_keys; i > 0; i--) {
        current->car_data[i] = current->car_data[i - 1];
    }
    if (!current->isleaf) {
        for (int i = current->car_num_keys + 1; i > 0; i--) {
            current->children[i] = current->children[i - 1];
        }
    }

    // Borrow from left
    current->car_data[0] = parent->car_data[index - 1];
    parent->car_data[index - 1] = left->car_data[left->car_num_keys - 1];

    if (!left->isleaf) {
        current->children[0] = left->children[left->car_num_keys];
    }

    left->car_num_keys--;
    current->car_num_keys++;
}

// Borrow from Right Sibling
void borrow_from_right(car_node* parent, int index) {
    car_node* right = parent->children[index + 1];
    car_node* current = parent->children[index];

    // Move parent's key down
    current->car_data[current->car_num_keys] = parent->car_data[index];
    parent->car_data[index] = right->car_data[0];

    // Shift keys in right sibling
    for (int i = 0; i < right->car_num_keys - 1; i++) {
        right->car_data[i] = right->car_data[i + 1];
    }

    if (!right->isleaf) {
        current->children[current->car_num_keys + 1] = right->children[0];
        for (int i = 0; i < right->car_num_keys; i++) {
            right->children[i] = right->children[i + 1];
        }
    }

    current->car_num_keys++;
    right->car_num_keys--;
}

// Delete a Car from the B-tree
void delete_car(car_node* node, char VIN[]) {
    int i = 0;
    while (i < node->car_num_keys && strcmp(VIN, node->car_data[i].car_VIN) > 0) {
        i++;
    }

    if (i < node->car_num_keys && strcmp(VIN, node->car_data[i].car_VIN) == 0) {
        if (node->isleaf) {
            for (int j = i; j < node->car_num_keys - 1; j++) {
                node->car_data[j] = node->car_data[j + 1];
            }
            node->car_num_keys--;
        } else {
            if (node->children[i]->car_num_keys >= (ORDER / 2)) {
                car predecessor = get_predecessor(node->children[i]);
                node->car_data[i] = predecessor;
                delete_car(node->children[i], predecessor.car_VIN);
            } else if (node->children[i + 1]->car_num_keys >= (ORDER / 2)) {
                car successor = get_successor(node->children[i + 1]);
                node->car_data[i] = successor;
                delete_car(node->children[i + 1], successor.car_VIN);
            } else {
                merge_nodes(node, i);
                delete_car(node->children[i], VIN);
            }
        }
    } else {
        if (node->isleaf) {
            printf("Car with VIN %s not found!\n", VIN);
            return;
        }

        boolean last_child = (i == node->car_num_keys);

        if (node->children[i]->car_num_keys < (ORDER / 2)) {
            if (i > 0 && node->children[i - 1]->car_num_keys >= (ORDER / 2)) {
                borrow_from_left(node, i);
            } else if (i < node->car_num_keys && node->children[i + 1]->car_num_keys >= (ORDER / 2)) {
                borrow_from_right(node, i);
            } else {
                if (i < node->car_num_keys) {
                    merge_nodes(node, i);
                } else {
                    merge_nodes(node, i - 1);
                }
            }
        }

        if (last_child && i > node->car_num_keys) {
            delete_car(node->children[i - 1], VIN);
        } else {
            delete_car(node->children[i], VIN);
        }
    }
}


status_code find_top_salespersons(sales_person_node *root, float *max_sales,
    sales_person **top_salespersons, int *top_count) {
if (root == NULL) return FAILURE;

for (int i = 0; i <= root->sales_person_num_keys; i++) {
// Check current key
if (i < root->sales_person_num_keys) {
sales_person *sp = &root->sales_person_data[i];

if (sp->sales_achived > *max_sales) {
*max_sales = sp->sales_achived;
*top_count = 0;
top_salespersons[(*top_count)++] = sp;
} else if (sp->sales_achived == *max_sales) {
if (*top_count < MAX)
top_salespersons[(*top_count)++] = sp;
}
}

// Recurse into children
if (!root->isleaf && root->children[i]) {
find_top_salespersons(root->children[i], max_sales, top_salespersons, top_count);
}
}

return (*top_count > 0) ? SUCCESS : FAILURE;
}

// Print top performers
status_code print_top_performers(sales_person **top_salespersons, int top_count, float max_sales) {
if (top_count == 0) {
printf("No top performers found.\n");
return FAILURE;
}

printf("\n=== Top Sales Performers ===\n");
for (int i = 0; i < top_count; i++) {
float incentive = 0.01f * top_salespersons[i]->sales_achived;
printf("\n--- Performer #%d ---\n", i + 1);
printf("ID        : %s\n", top_salespersons[i]->sales_person_ID);
printf("Name      : %s\n", top_salespersons[i]->sales_person_name);
printf("Sales     : %.2f\n", top_salespersons[i]->sales_achived);
printf("Incentive : %.2f (1%% bonus)\n", incentive);
}

return SUCCESS;
}
car_node* search_car_node_by_vin(car_node *root, char *vin) {
    if (root == NULL) return NULL;

    int i = 0;
    while (i < root->car_num_keys && strcmp(vin, root->car_data[i].car_VIN) > 0) {
        i++;
    }

    if (i < root->car_num_keys && strcmp(vin, root->car_data[i].car_VIN) == 0) {
        return root;  // This node contains the matching car
    }

    if (root->isleaf) {
        return NULL;  // Not found
    } else {
        return search_car_node_by_vin(root->children[i], vin);
    }
}


void display_car_details_by_VIN(char* VIN, car_node* available_root, car_node* sold_root) {
    car_node* found = NULL;

    // Search in available (unsold) cars first
    found = search_car_node_by_vin(available_root, VIN);
    if (found != NULL && found->car_data != NULL) {
        printf("\n--- Car Found in Available Stock ---\n");
        printf("Car Name      : %s\n", found->car_data->car_name);
        printf("VIN           : %s\n", found->car_data->car_VIN);
        printf("Color         : %s\n", found->car_data->car_colour);
        printf("Fuel Type     : %s\n", found->car_data->fuel_type);
        printf("Car Type      : %s\n", found->car_data->car_type);
        printf("Price         : %.2f\n", found->car_data->car_price);
        printf("Status        : In Stock\n");
        return;
    }

    // Search in sold cars if not found in available
    found = search_car_node_by_vin(sold_root, VIN);
    if (found != NULL && found->car_data != NULL) {
        printf("\n--- Car Found in Sold Cars ---\n");
        printf("Car Name      : %s\n", found->car_data->car_name);
        printf("VIN           : %s\n", found->car_data->car_VIN);
        printf("Color         : %s\n", found->car_data->car_colour);
        printf("Fuel Type     : %s\n", found->car_data->fuel_type);
        printf("Car Type      : %s\n", found->car_data->car_type);
        printf("Price         : %.2f\n", found->car_data->car_price);
        printf("Sold To       : Customer ID - %s\n", found->car_data->car_customer_ID);
        printf("Sold By       : Salesperson ID - %s\n", found->car_data->car_sales_person_ID);
        printf("Status        : Sold\n");
        return;
    }

    // If not found in either tree
    printf("\nCar with VIN %s not found in system.\n", VIN);
}


void search_salespersons_in_range(sales_person_node* root, float min_sales, float max_sales) {
    if (root == NULL) return;

    int i;
    for (i = 0; i < root->sales_person_num_keys; i++) {
        // Check child before this key
        if (root->children[i] != NULL) {
            search_salespersons_in_range(root->children[i], min_sales, max_sales);
        }

        sales_person* sp =&( root->sales_person_data[i]);
        if (sp->sales_achived >= min_sales && sp->sales_achived <= max_sales) {
            printf("\n--- Salesperson in Range ---\n");
            printf("ID                : %s\n", sp->sales_person_ID);
            printf("Name              : %s\n", sp->sales_person_name);
            printf("Sales Target      : %.2f\n", sp->sales_target);
            printf("Sales Achieved    : %.2f\n", sp->sales_achived);
            printf("Commission (%%)    : %.2f\n", sp->commission_per_sale);
        }
    }

    // Check last child
    if (root->children[i] != NULL) {
        search_salespersons_in_range(root->children[i], min_sales, max_sales);
    }
}




// Modified sell_car_to_customer function
int sell_car_to_customer(showroom *target_showroom,  char *VIN_to_sell) {
    if (!target_showroom) {
        printf("Invalid showroom!\n");
        return 0;
    }

    // Search car in available inventory
    car_node* car_to_sell = search_car_node_by_vin(
        target_showroom->avialable_cars_root->car_children, 
        VIN_to_sell
    );

    if (!car_to_sell) {
        printf("Car %s not found in available stock!\n", VIN_to_sell);
        return 0;
    }

    // Collect customer details
    char customer_id[SIZE], customer_name[SIZE], mobile[11], address[SIZE], reg_no[SIZE], payment_type[SIZE],purchase_date[SIZE];
    int loan_period;
    printf("Enter Customer ID: ");
    scanf("%s", customer_id);
    printf("Enter Customer Name: ");
    scanf(" %[^\n]", customer_name);
    printf("Enter Mobile Number: ");
    scanf("%s", mobile);
    printf("Enter Address: ");
    scanf(" %[^\n]", address);
    printf("Enter Registration Number: ");
    scanf("%s", reg_no);
    
    while(1) {
        printf("Enter payment type (Cash/Loan): ");
        scanf("%s", payment_type);
        
        if (strcmp(payment_type, "Cash") == 0 || strcmp(payment_type, "Loan") == 0) {
            break;
        }
        printf("Invalid payment type! Please enter either 'Cash' or 'Loan'.\n");
    }

    // Handle loan period if payment type is Loan
    if (strcmp(payment_type, "Loan") == 0) {
        printf("\nAvailable loan options:\n");
        printf("1. 9.00%% rate of interest for 84 months EMI\n");
        printf("2. 8.75%% rate of interest for 60 months EMI\n");
        printf("3. 8.50%% rate of interest for 36 months EMI\n");
        
        int option;
        while(1) {
            printf("Select loan option (1-3): ");
            scanf("%d", &option);
            
            if (option >= 1 && option <= 3) {
                break;
            }
            printf("Invalid option! Please select 1, 2, or 3.\n");
        }
        
        switch(option) {
            case 1: loan_period = 84; break;
            case 2: loan_period = 60; break;
            case 3: loan_period = 36; break;
        }
    }
    else {
        loan_period = 0;
    }

    printf("Enter Purchase Date (YYYY-MM-DD): ");
    scanf("%s", purchase_date);

    // Salesperson handling
    char sales_person_ID[SIZE];
    printf("Enter Salesperson ID: ");
    scanf("%s", sales_person_ID);
    
    sales_person *sp = search_salesperson_by_id(
        target_showroom->salespersons_root->sales_person_children,
        sales_person_ID
    );

    if (!sp) {
        printf("Salesperson not found! Creating new...\n");
        char sales_person_name[SIZE];
        printf("Enter Salesperson Name: ");
        scanf(" %[^\n]", sales_person_name);

        // Initialize new salesperson
        sp = malloc(sizeof(sales_person));
        strcpy(sp->sales_person_ID, sales_person_ID);
        strcpy(sp->sales_person_name, sales_person_name);
        sp->sales_target = 0;
        sp->sales_achived = 0;
        sp->commission_per_sale = 0;
        sp->customer_tree_root = malloc(sizeof(customer_root));
        sp->customer_tree_root->customer_children = NULL;
        sp->sold_car_root = NULL;

        // Insert into showroom's salesperson tree
        if (!insert_salesperson(
            &target_showroom->salespersons_root->sales_person_children,
            sp->sales_person_ID,
            sp->sales_person_name,
            0, 0,
            customer_id, customer_name, mobile, address, reg_no, 
            VIN_to_sell,payment_type, loan_period,
            purchase_date)
        ){
            free(sp);
            return 0;
        }
    }

    // Update salesperson records
    sp->sales_achived += car_to_sell->car_data->car_price;
    sp->commission_per_sale = 0.02 * sp->sales_achived;
    status_code sc = insert_customer(&sp->customer_tree_root->customer_children,customer_id, customer_name, mobile,
        address, reg_no, VIN_to_sell);
    status_code sc1 = insert_sold_car(&sp->sold_car_root, VIN_to_sell, payment_type, loan_period, purchase_date);

    // Move car from available to sold
    if (!insert_car(
        &target_showroom->sold_cars_root->car_children,
        VIN_to_sell,
        car_to_sell->car_data->car_name,
        car_to_sell->car_data->car_type,
        car_to_sell->car_data->car_colour,
        car_to_sell->car_data->fuel_type,
        car_to_sell->car_data->car_price,
        customer_id,
        sales_person_ID,
        purchase_date // Actual date should be collected
    )) {
        return 0;
    }

    // Delete from available cars
    delete_car(target_showroom->avialable_cars_root->car_children, VIN_to_sell);

    printf("Sale completed successfully!\n");
    return 1;
}

void write_car_btree(FILE *file, car_node *node) {
    if (node == NULL) return;
    
    // Write all keys in this node
    for (int i = 0; i < node->car_num_keys; i++) {
        fprintf(file, "%s,%s,%s,%s,%s,%.2f,%s,%s,%s\n",
                node->car_data[i].car_name,
                node->car_data[i].car_VIN,
                node->car_data[i].car_type,
                node->car_data[i].car_colour,
                node->car_data[i].fuel_type,
                node->car_data[i].car_price,
                node->car_data[i].car_customer_ID,
                node->car_data[i].car_sales_person_ID,
                node->car_data[i].purchase_date);
    }
    
    // Recursively write children if not leaf
    if (!node->isleaf) {
        for (int i = 0; i <= node->car_num_keys; i++) {
            write_car_btree(file, node->children[i]);
        }
    }
}

void write_salesperson_records(FILE *file, sales_person_node *node) {
    if (node == NULL) return;
    
    // Process all salespersons in this node
    for (int i = 0; i < node->sales_person_num_keys; i++) {
        sales_person *sp = &node->sales_person_data[i];
        
        // Write a record for each customer
        if (sp->customer_tree_root && sp->customer_tree_root->customer_children) {
            customer_node *cust_node = sp->customer_tree_root->customer_children;
            
            // Stack for B-tree traversal
            customer_node *stack[100];
            int top = -1;
            stack[++top] = cust_node;
            
            while (top >= 0) {
                customer_node *current = stack[top--];
                
                // Process all customers in this node
                for (int j = 0; j < current->customer_num_keys; j++) {
                    // Get payment details from sold_car tree
                    char Payment_Type[SIZE] = "Cash";
                    int loan_period = 0;
                    char purchase_date[SIZE] = "2025-01-01";
                    
                    // Find the sold car details
                    if (sp->sold_car_root) {
                        sold_car_node *car_node = sp->sold_car_root;
                        int found = 0;
                        
                        // Search for the VIN in sold_car B-tree
                        while (car_node && !found) {
                            for (int k = 0; k < car_node->sold_car_num_keys; k++) {
                                if (strcmp(car_node->sold_car[k].VIN, 
                                          current->customer_data[j].sold_car_VIN) == 0) {
                                    strcpy(Payment_Type, car_node->sold_car[k].Payment_Type);
                                    loan_period = car_node->sold_car[k].loan_period;
                                    strcpy(purchase_date, car_node->sold_car[k].purchase_date);
                                    found = 1;
                                    break;
                                }
                            }
                            if (!found && !car_node->isleaf) {
                                // Determine which child to search next
                                int l;
                                for (l = 0; l < car_node->sold_car_num_keys; l++) {
                                    if (strcmp(current->customer_data[j].sold_car_VIN, 
                                              car_node->sold_car[l].VIN) < 0) {
                                        break;
                                    }
                                }
                                car_node = car_node->children[l];
                            } else {
                                break;
                            }
                        }
                    }
                    
                    // Write the complete salesperson record
                    fprintf(file, "%s,%s,%.2f,%.2f,%s,%s,%s,%s,%s,%s,%s,%d,%s\n",
                            sp->sales_person_ID,
                            sp->sales_person_name,
                            sp->sales_target,
                            sp->sales_achived,
                            current->customer_data[j].customer_id,
                            current->customer_data[j].customer_name,
                            current->customer_data[j].mobile_no,
                            current->customer_data[j].customer_address,
                            current->customer_data[j].registration_number,
                            current->customer_data[j].sold_car_VIN,
                            Payment_Type,
                            loan_period,
                            purchase_date);
                }
                
                // Push children to stack if not leaf
                if (!current->isleaf) {
                    for (int j = current->customer_num_keys; j >= 0; j--) {
                        if (current->children[j]) {
                            stack[++top] = current->children[j];
                        }
                    }
                }
            }
        }
    }
    
    // Recursively process child nodes
    if (!node->isleaf) {
        for (int i = 0; i <= node->sales_person_num_keys; i++) {
            write_salesperson_records(file, node->children[i]);
        }
    }
}

void update_showroom_file(const char *filename, const showroom *sr) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error opening file %s for writing\n", filename);
        return;
    }

    // 1. Write showroom metadata
    fprintf(file, "%s\n", sr->showroom_id);
    fprintf(file, "%s\n", sr->avialable_cars_root->unique_id);

    // 2. Write all cars (first sold, then available)
    if (sr->sold_cars_root && sr->sold_cars_root->car_children) {
        write_car_btree(file, sr->sold_cars_root->car_children);
    }
    if (sr->avialable_cars_root && sr->avialable_cars_root->car_children) {
        write_car_btree(file, sr->avialable_cars_root->car_children);
    }

    // 3. Write SalesPerson section
    fprintf(file, "SalesPerson\n");
    if (sr->salespersons_root && sr->salespersons_root->sales_person_children) {
        write_salesperson_records(file, sr->salespersons_root->sales_person_children);
    }

    fclose(file);
}


void display_showroom(showroom show)
{
    printf("The showroom data\n");
    printf("---------------------------------------------------\n");
    printf("The available cars in showroom :\n");
    car_node *temp = show.avialable_cars_root->car_children;
    traverse_car_tree(temp);
    printf("---------------------------------------------------\n");
    printf("The sold cars in showroom :\n");
    car_node *temp1 = show.sold_cars_root->car_children;
    traverse_car_tree(temp1);
    printf("---------------------------------------------------\n");
    printf("The salesperson in showroom :\n");
    sales_person_node *temp3 = show.salespersons_root->sales_person_children;
    traverse_salesperson_tree(temp3);
    printf("\n");
    printf("%d\n", show.available_cars);
    printf("%d\n", show.sold_cars);
}


int main()
{
    const char *filenames[] = {"showroom1.txt", "showroom2.txt", "showroom3.txt"};
    showroom showrooms[3];

    for (int i = 0; i < 3; i++)
    {
        creating_showroom(filenames[i], &showrooms[i]);
    }
    /*for (int i = 0; i < 3; i++)
    {
        display_showroom(showrooms[i]);
    }
    printf("\n");
    for (int i = 0; i < 3; i++)
    {
        printf("%s\n",showrooms[i].avialable_cars_root->unique_id);
        printf("%s\n",showrooms[i].sold_cars_root->unique_id);
        printf("%s\n",showrooms[i].salespersons_root->showroom_id);
    }*/

    showroom merged = merge_showrooms(showrooms,3);
    display_showroom(merged);
   

    int choice = 1 ;
    while(choice)
    {
        int flag ;
        printf("Choose 2 input details of sales person to add new sales person\n");
        printf("Choose 3 To Find the most popular car among all three showrooms\n");
        printf("Choose 4 Find the most successful sales person according to sales. Award him/her with 1percentage"
             "extra incentives as per sales achieved and print the same\n");
        printf("Choose 5 To input details of sales person and customer to sale the car to customer.\n");
        printf("Choose 6  Based on previous month's sales figures, To predict the next month sales\n");
        printf("Choose 7 To display all information of car even if it is in stock or sold\n");
        printf("Choose 8 To Search the range of sales person who have achieved sales target within given range of values (min_sales, max_sales).\n");
        printf("Choose 9 To Print list of customers having EMI plan for less than 48 months but greater than 36 months.\n");
        scanf("%d",&flag);
        getchar();
        switch(flag){
            case (2):
            {
            char showroom_id[SIZE];
            char sales_person_ID[SIZE];
            char sales_person_name[SIZE];
            float sales_target;
            printf("Enter  showroom_id in whihc you want to add the sales person : \n");
            fgets(showroom_id, sizeof(showroom_id), stdin);
            showroom_id[strcspn( showroom_id, "\n")] = '\0';
            printf("Enetr his name : \n");
            fgets(sales_person_name, sizeof(sales_person_name), stdin);
            sales_person_name[strcspn( sales_person_name, "\n")] = '\0';
            printf("Enter his id : \n");
            fgets(sales_person_ID, sizeof(sales_person_ID), stdin);
            sales_person_ID[strcspn(sales_person_ID, "\n")] = '\0';
            printf("enter his sales target : \n");
            scanf("%f",&sales_target);
            getchar();

            int found =1;
            for(int i=0;i<(3 && found);i++){
                if(strcmp(showrooms[i].showroom_id ,showroom_id) == 0 ){
                    status_code sc1 = insert_salesperson(&(merged.salespersons_root->sales_person_children),
                    sales_person_ID, sales_person_name,sales_target,0.0,"","","","","","","",0,"");
                    status_code sc =insert_salesperson(&(showrooms[i].salespersons_root->sales_person_children),sales_person_ID, 
                    sales_person_name,sales_target,0.0,"","","","","","","",0,"");
                    if(sc== SUCCESS){
                        printf("insertion done\n");
                        traverse_salesperson_tree(showrooms[i].salespersons_root->sales_person_children);
                        traverse_salesperson_tree(merged.salespersons_root->sales_person_children);
                    }
                    else{
                        printf("insertion failed\n");
                    }
                    found =0;
                }
                else{
                    printf("enter correct showroom id\n");
                }
            }
            break;
            }
            case (3):
            {
            find_most_popular_car_in_showroom(merged.sold_cars_root->car_children);
            break;
            }
            case (4):
            {
                sales_person *top_salespersons[MAX];
                int top_count = 0;
                float max_sales = -1.0f;
                
                status_code result = find_top_salespersons(merged.salespersons_root->sales_person_children, &max_sales, top_salespersons, &top_count);
                
                if (result == SUCCESS) {
                    print_top_performers(top_salespersons, top_count, max_sales);
                } else {
                    printf(" No salespersons found in the tree.\n");
                }
             break;
            }
            case 5: {
                char showroom_id[SIZE];
                printf("Enter Showroom ID: ");
                scanf("%s", showroom_id);
                
                // Find correct showroom
                int showroom_index = -1;
                for (int i = 0; i < 3; i++) {
                    if (strcmp(showrooms[i].showroom_id, showroom_id) == 0) {
                        showroom_index = i;
                        break;
                    }
                }
                
                if (showroom_index == -1) {
                    printf("Invalid showroom ID!\n");
                    break;
                }
            
                char VIN_to_sell[SIZE];
                printf("Enter VIN of car to sell: ");
                scanf("%s", VIN_to_sell);
            
                int result = sell_car_to_customer(&showrooms[showroom_index], VIN_to_sell);
                
                if (result) {
                    printf("Car sold successfully!\n");
                    // Update counts
                    showrooms[showroom_index].available_cars--;
                    showrooms[showroom_index].sold_cars++;
                    //int result2 = sell_car_to_customer(&merged, VIN_to_sell);
                    display_showroom(showrooms[showroom_index]);
                    //display_showroom(merged);
                } else {
                    printf("Failed to complete sale!\n");
                }
                break;
            }
           
            case (6):
            {
            predict_next_month_sales(merged.sold_cars_root->car_children);
            break;
            }
            case (7):
            {
            char vin_input[20];
            printf("Enter VIN to search: ");
            scanf("%s", vin_input);
            getchar();

            display_car_details_by_VIN(vin_input, merged.avialable_cars_root->car_children, merged.sold_cars_root->car_children);
            break;
            }
            case (8):
            {
            float min_sales, max_sales;

            printf("Enter minimum sales achieved: ");
            scanf("%f", &min_sales);

            printf("Enter maximum sales achieved: ");
            scanf("%f", &max_sales);
            getchar();

            // Now call your function
            search_salespersons_in_range(merged.salespersons_root->sales_person_children, min_sales, max_sales);
            break;
            }
            case (9):
            {
            traverse_salesperson_and_print(merged.salespersons_root->sales_person_children);
            break;
            }
            default:
            printf("Invalid option. Please enter a valid number between 2 and 9.\n");
            break;
        }
        printf("For continuing enter 1 else enter 0\n ");
        scanf("%d",&choice);
        getchar();
    }
    
        printf("\nUpdating all showroom files...\n");
        for (int i = 0; i < 3; i++) {
            update_showroom_file(filenames[i], &showrooms[i]);
            printf("Updated %s\n", filenames[i]);
        }
        
        // Also update the merged showroom if you want to save it
        // update_showroom_file("merged_showroom.txt", &merged);
    
        printf("All files updated successfully.\n");
        
    return 0;
}
