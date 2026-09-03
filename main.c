#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define STAT_MAX 10U
#define LOW_WARNING 3U
#define NAME_LEN 12U

/* mood bits */
#define BIT_ASLEEP 0U
#define BIT_HUNGRY 1U
#define BIT_SAD 2U
#define BIT_SICK 3U

#define SET_BIT(reg, n) ((reg) |= (uint8_t)(1U << (n)))
#define CLR_BIT(reg, n) ((reg) &= (uint8_t)~(1U << (n)))
#define READ_BIT(reg, n) ((uint8_t)(((reg) >> (n)) & 1U))

typedef struct {
    char name[NAME_LEN];
    uint8_t food;     /* 0..10 */
    uint8_t fun;      /* 0..10 */
    uint8_t energy;   /* 0..10 */
    uint8_t mood;     /* the bits above */
    uint16_t hours;   /* how long Fluffy has been alive */
} Pet_t;

static Pet_t fluffy;


/* Prototypes */
static void hatchPet(void);
static void drawFace(void);
static void drawStat(const char *label, uint8_t value);
static void feed(void);
static void play(void);
static void sleepPet(void);
static void hourPasses(void);
static void updateMood(void);
static uint8_t isHappy(void);
static void petReport(void);


/* =========================================================
   hatchPet
   ========================================================= */
static void hatchPet(void)
{
    printf("Enter Fluffy's name: ");

    if (scanf("%11s", fluffy.name) != 1)
    {
        strcpy(fluffy.name, "Fluffy");
    }

    fluffy.food = (uint8_t)STAT_MAX;
    fluffy.fun = (uint8_t)STAT_MAX;
    fluffy.energy = (uint8_t)STAT_MAX;

    fluffy.mood = 0U;
    fluffy.hours = 0U;

    CLR_BIT(fluffy.mood, BIT_ASLEEP);
    CLR_BIT(fluffy.mood, BIT_HUNGRY);
    CLR_BIT(fluffy.mood, BIT_SAD);
    CLR_BIT(fluffy.mood, BIT_SICK);

    printf("%s has hatched!\n", fluffy.name);
}


/* =========================================================
   drawFace
   Priority:
   asleep -> sick -> sad -> happy
   ========================================================= */
static void drawFace(void)
{
    printf("\n+-------+\n");

    if (READ_BIT(fluffy.mood, BIT_ASLEEP) != 0U)
    {
        printf("| -   - |\n");
        printf("|   -   |\n");
        printf("|  ___  |\n");
    }
    else if (READ_BIT(fluffy.mood, BIT_SICK) != 0U)
    {
        printf("| x   x |\n");
        printf("|   ~   |\n");
        printf("|  ___  |\n");
    }
    else if (READ_BIT(fluffy.mood, BIT_SAD) != 0U)
    {
        printf("| o   o |\n");
        printf("|   .   |\n");
        printf("|  \\_/  |\n");
    }
    else
    {
        printf("| ^   ^ |\n");
        printf("|   .   |\n");
        printf("|  \\_/  |\n");
    }

    printf("+-------+\n");
}


/* =========================================================
   drawStat
   ========================================================= */
static void drawStat(const char *label, uint8_t value)
{
    uint8_t i;

    printf("%-7s ", label);

    for (i = 0U; i < STAT_MAX; i++)
    {
        if (i < value)
        {
            printf("[#]");
        }
        else
        {
            printf("[ ]");
        }
    }

    printf(" %u\n", (unsigned int)value);
}


/* =========================================================
   feed
   ========================================================= */
static void feed(void)
{
    uint8_t oldFood;

    if (READ_BIT(fluffy.mood, BIT_ASLEEP) != 0U)
    {
        printf("Fluffy is sleeping. Wake Fluffy first.\n");
        return;
    }

    oldFood = fluffy.food;

    if (fluffy.food < STAT_MAX)
    {
        if (fluffy.food > (STAT_MAX - 3U))
        {
            fluffy.food = (uint8_t)STAT_MAX;
        }
        else
        {
            fluffy.food = (uint8_t)(fluffy.food + 3U);
        }
    }
    else
    {
        fluffy.food = (uint8_t)STAT_MAX;
    }

    /*
     * Eating when food was already full makes Fluffy sick.
     */
    if (oldFood == STAT_MAX)
    {
        SET_BIT(fluffy.mood, BIT_SICK);
    }
    else
    {
        /*
         * Do not clear SICK here.
         * Sick only goes away after sleeping.
         */
        if (READ_BIT(fluffy.mood, BIT_SICK) != 0U)
        {
            SET_BIT(fluffy.mood, BIT_SICK);
        }
        else
        {
            CLR_BIT(fluffy.mood, BIT_SICK);
        }
    }

    updateMood();

    printf("Fluffy ate some food.\n");
}


/* =========================================================
   play
   ========================================================= */
static void play(void)
{
    if (READ_BIT(fluffy.mood, BIT_ASLEEP) != 0U)
    {
        printf("Fluffy is sleeping and cannot play.\n");
        return;
    }

    if (fluffy.fun > (STAT_MAX - 3U))
    {
        fluffy.fun = (uint8_t)STAT_MAX;
    }
    else
    {
        fluffy.fun = (uint8_t)(fluffy.fun + 3U);
    }

    if (fluffy.energy < 2U)
    {
        fluffy.energy = 0U;
    }
    else
    {
        fluffy.energy = (uint8_t)(fluffy.energy - 2U);
    }

    updateMood();

    printf("Fluffy played happily.\n");
}


/* =========================================================
   sleepPet
   ========================================================= */
static void sleepPet(void)
{
    if (READ_BIT(fluffy.mood, BIT_ASLEEP) != 0U)
    {
        /*
         * Wake Fluffy.
         */
        CLR_BIT(fluffy.mood, BIT_ASLEEP);

        /*
         * Sickness disappears only after Fluffy has slept.
         */
        CLR_BIT(fluffy.mood, BIT_SICK);

        printf("Fluffy woke up.\n");
    }
    else
    {
        /*
         * Put Fluffy to sleep.
         */
        SET_BIT(fluffy.mood, BIT_ASLEEP);

        printf("Fluffy is now sleeping.\n");
    }

    updateMood();
}


/* =========================================================
   hourPasses
   ========================================================= */
static void hourPasses(void)
{
    /*
     * Food decreases by 1, never below zero.
     */
    if (fluffy.food > 0U)
    {
        fluffy.food--;
    }
    else
    {
        fluffy.food = 0U;
    }

    /*
     * Fun decreases by 1, never below zero.
     */
    if (fluffy.fun > 0U)
    {
        fluffy.fun--;
    }
    else
    {
        fluffy.fun = 0U;
    }

    /*
     * Energy:
     * awake  -> -1
     * asleep -> +2
     */
    if (READ_BIT(fluffy.mood, BIT_ASLEEP) != 0U)
    {
        if (fluffy.energy > (STAT_MAX - 2U))
        {
            fluffy.energy = (uint8_t)STAT_MAX;
        }
        else
        {
            fluffy.energy = (uint8_t)(fluffy.energy + 2U);
        }
    }
    else
    {
        if (fluffy.energy > 0U)
        {
            fluffy.energy--;
        }
        else
        {
            fluffy.energy = 0U;
        }
    }

    /*
     * One hour has passed.
     */
    fluffy.hours++;

    /*
     * Recalculate mood after all changes.
     */
    updateMood();
}


/* =========================================================
   updateMood
   ========================================================= */
static void updateMood(void)
{
    /* Hungry */
    if (fluffy.food <= LOW_WARNING)
    {
        SET_BIT(fluffy.mood, BIT_HUNGRY);
    }
    else
    {
        CLR_BIT(fluffy.mood, BIT_HUNGRY);
    }

    /* Sad */
    if (fluffy.fun <= LOW_WARNING)
    {
        SET_BIT(fluffy.mood, BIT_SAD);
    }
    else
    {
        CLR_BIT(fluffy.mood, BIT_SAD);
    }

    /*
     * No energy means Fluffy falls asleep automatically.
     */
    if (fluffy.energy == 0U)
    {
        SET_BIT(fluffy.mood, BIT_ASLEEP);
    }
    else
    {
        /*
         * Do not wake Fluffy just because energy is available.
         * If asleep, Fluffy stays asleep until sleepPet() wakes it.
         */
        if (READ_BIT(fluffy.mood, BIT_ASLEEP) != 0U)
        {
            SET_BIT(fluffy.mood, BIT_ASLEEP);
        }
        else
        {
            CLR_BIT(fluffy.mood, BIT_ASLEEP);
        }
    }

    /*
     * SICK is intentionally NOT recalculated here.
     * It is cleared only after Fluffy has slept.
     */
}


/* =========================================================
   isHappy
   ========================================================= */
static uint8_t isHappy(void)
{
    if ((fluffy.food > LOW_WARNING) &&
        (fluffy.fun > LOW_WARNING) &&
        (fluffy.energy > LOW_WARNING) &&
        (READ_BIT(fluffy.mood, BIT_HUNGRY) == 0U) &&
        (READ_BIT(fluffy.mood, BIT_SAD) == 0U) &&
        (READ_BIT(fluffy.mood, BIT_SICK) == 0U) &&
        (READ_BIT(fluffy.mood, BIT_ASLEEP) == 0U))
    {
        return 1U;
    }
    else
    {
        return 0U;
    }
}


/* =========================================================
   petReport
   ========================================================= */
static void petReport(void)
{
    uint8_t bit;

    printf("\n====================================\n");
    printf("           FLUFFY CARE CARD\n");
    printf("====================================\n");

    printf("Name        : %s\n", fluffy.name);
    printf("Hours alive : %u\n", (unsigned int)fluffy.hours);

    printf("\n");
    drawStat("Food", fluffy.food);
    drawStat("Fun", fluffy.fun);
    drawStat("Energy", fluffy.energy);

    printf("\nMood byte   : ");

    for (bit = 7U; bit > 0U; bit--)
    {
        printf("%u", (unsigned int)READ_BIT(fluffy.mood, bit));
    }

    printf("%u\n",
           (unsigned int)READ_BIT(fluffy.mood, 0U));

    if (READ_BIT(fluffy.mood, BIT_ASLEEP) != 0U)
    {
        printf("Fluffy needs: sleep and rest.\n");
    }
    else if (READ_BIT(fluffy.mood, BIT_SICK) != 0U)
    {
        printf("Fluffy needs: sleep to recover.\n");
    }
    else if (READ_BIT(fluffy.mood, BIT_HUNGRY) != 0U)
    {
        printf("Fluffy needs: food.\n");
    }
    else if (READ_BIT(fluffy.mood, BIT_SAD) != 0U)
    {
        printf("Fluffy needs: some play time.\n");
    }
    else
    {
        printf("Fluffy needs: nothing - Fluffy is happy!\n");
    }

    printf("====================================\n");
}

//هذه الدالة انا عملتها عشا اقدر اعمل الRun

int main(void)
{
    int choice;
    hatchPet();

    do {
        printf("\n--- Fluffy Test Menu ---\n");
        printf("1. Show Face & Report\n");
        printf("2. Feed Fluffy\n");
        printf("3. Play with Fluffy\n");
        printf("4. Sleep / Wake up\n");
        printf("5. Hour passes\n");
        printf("6. Exit\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1)
        {
            break;
        }

        switch (choice)
        {
            case 1:
                drawFace();
                petReport();
                break;
            case 2:
                feed();
                break;
            case 3:
                play();
                break;
            case 4:
                sleepPet();
                break;
            case 5:
                hourPasses();
                printf("1 hour passed in Fluffy's world.\n");
                break;
            case 6:
                printf("Exiting test mode.\n");
                break;
            default:
                printf("Invalid choice. Try again.\n");
                break;
        }
    } while (choice != 6);

    return 0;
}
