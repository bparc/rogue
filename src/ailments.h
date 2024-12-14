typedef enum
{
    status_effect_none,
    status_effect_poison,
    status_effect_freeze,
    status_effect_stun,
    status_effect_Count,
} status_effect_type_t;

const v4 status_effect_Colors[status_effect_Count] =
{
    {1.0f, 1.0f, 1.0f, 1.0f},
    {0.0f, 1.0f, 0.0f, 1.0f}, // status_effect_poison
    {0.0f, 0.0f, 1.0f, 1.0f}, // status_effect_freeze
    {1.0f, 0.0f, 1.0f, 1.0f}, // status_effect_stun
};

const char *status_effect_Text[status_effect_Count] =
{
    "NONE", "POISON", "FREEZE", "STUN",
};

typedef struct
{
    status_effect_type_t type;
    s32 duration;
} status_effect_t;
