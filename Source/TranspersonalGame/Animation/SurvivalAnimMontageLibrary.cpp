#include "SurvivalAnimMontageLibrary.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"

// Static member initialization
TMap<EAnim_SurvivalMontageType, FAnim_MontageConfig> USurvivalAnimMontageLibrary::MontageConfigs;

bool USurvivalAnimMontageLibrary::PlaySurvivalMontage(UAnimInstance* AnimInstance, EAnim_SurvivalMontageType MontageType, float PlayRate)
{
    if (!AnimInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("USurvivalAnimMontageLibrary::PlaySurvivalMontage - Invalid AnimInstance"));
        return false;
    }

    if (MontageType == EAnim_SurvivalMontageType::None)
    {
        UE_LOG(LogTemp, Warning, TEXT("USurvivalAnimMontageLibrary::PlaySurvivalMontage - MontageType is None"));
        return false;
    }

    // Initialize configs if needed
    if (MontageConfigs.Num() == 0)
    {
        InitializeMontageConfigs();
    }

    // Get montage configuration
    FAnim_MontageConfig* Config = MontageConfigs.Find(MontageType);
    if (!Config)
    {
        UE_LOG(LogTemp, Warning, TEXT("USurvivalAnimMontageLibrary::PlaySurvivalMontage - No config found for montage type"));
        return false;
    }

    // Load montage asset
    UAnimMontage* Montage = Config->MontageSoft.LoadSynchronous();
    if (!Montage)
    {
        UE_LOG(LogTemp, Warning, TEXT("USurvivalAnimMontageLibrary::PlaySurvivalMontage - Failed to load montage asset"));
        return false;
    }

    // Stop any currently playing montage
    if (AnimInstance->IsAnyMontagePlaying())
    {
        AnimInstance->StopAllMontages(Config->BlendOutTime);
    }

    // Play the montage
    float MontageLength = AnimInstance->Montage_Play(Montage, PlayRate * Config->PlayRate);
    if (MontageLength > 0.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("USurvivalAnimMontageLibrary::PlaySurvivalMontage - Playing montage %s"), *Montage->GetName());
        return true;
    }

    UE_LOG(LogTemp, Warning, TEXT("USurvivalAnimMontageLibrary::PlaySurvivalMontage - Failed to play montage"));
    return false;
}

void USurvivalAnimMontageLibrary::StopSurvivalMontage(UAnimInstance* AnimInstance, float BlendOutTime)
{
    if (!AnimInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("USurvivalAnimMontageLibrary::StopSurvivalMontage - Invalid AnimInstance"));
        return;
    }

    if (AnimInstance->IsAnyMontagePlaying())
    {
        AnimInstance->StopAllMontages(BlendOutTime);
        UE_LOG(LogTemp, Log, TEXT("USurvivalAnimMontageLibrary::StopSurvivalMontage - Stopped all montages"));
    }
}

bool USurvivalAnimMontageLibrary::IsSurvivalMontageActive(UAnimInstance* AnimInstance)
{
    if (!AnimInstance)
    {
        return false;
    }

    return AnimInstance->IsAnyMontagePlaying();
}

EAnim_SurvivalMontageType USurvivalAnimMontageLibrary::GetCurrentSurvivalMontageType(UAnimInstance* AnimInstance)
{
    if (!AnimInstance || !AnimInstance->IsAnyMontagePlaying())
    {
        return EAnim_SurvivalMontageType::None;
    }

    // Get currently playing montage
    UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage();
    if (!CurrentMontage)
    {
        return EAnim_SurvivalMontageType::None;
    }

    // Find montage type by comparing asset names
    for (const auto& ConfigPair : MontageConfigs)
    {
        UAnimMontage* ConfigMontage = ConfigPair.Value.MontageSoft.LoadSynchronous();
        if (ConfigMontage == CurrentMontage)
        {
            return ConfigPair.Key;
        }
    }

    return EAnim_SurvivalMontageType::None;
}

void USurvivalAnimMontageLibrary::ConfigureMontageSettings(EAnim_SurvivalMontageType MontageType, const FAnim_MontageConfig& Config)
{
    if (MontageType == EAnim_SurvivalMontageType::None)
    {
        return;
    }

    // Initialize configs if needed
    if (MontageConfigs.Num() == 0)
    {
        InitializeMontageConfigs();
    }

    MontageConfigs.Add(MontageType, Config);
    UE_LOG(LogTemp, Log, TEXT("USurvivalAnimMontageLibrary::ConfigureMontageSettings - Updated config for montage type"));
}

FAnim_MontageConfig USurvivalAnimMontageLibrary::GetMontageConfig(EAnim_SurvivalMontageType MontageType)
{
    if (MontageType == EAnim_SurvivalMontageType::None)
    {
        return FAnim_MontageConfig();
    }

    // Initialize configs if needed
    if (MontageConfigs.Num() == 0)
    {
        InitializeMontageConfigs();
    }

    FAnim_MontageConfig* Config = MontageConfigs.Find(MontageType);
    if (Config)
    {
        return *Config;
    }

    return FAnim_MontageConfig();
}

bool USurvivalAnimMontageLibrary::PlayContextualSurvivalAnimation(UAnimInstance* AnimInstance, float Health, float Hunger, float Thirst, float Fear)
{
    if (!AnimInstance)
    {
        return false;
    }

    // Determine appropriate animation based on survival stats
    EAnim_SurvivalMontageType ContextualMontage = EAnim_SurvivalMontageType::None;

    // Priority system for contextual animations
    if (Fear > 0.8f)
    {
        ContextualMontage = EAnim_SurvivalMontageType::Fearful;
    }
    else if (Health < 0.3f)
    {
        ContextualMontage = EAnim_SurvivalMontageType::Injured;
    }
    else if (Thirst > 0.9f)
    {
        ContextualMontage = EAnim_SurvivalMontageType::Drinking;
    }
    else if (Hunger > 0.9f)
    {
        ContextualMontage = EAnim_SurvivalMontageType::Eating;
    }
    else if (Health < 0.6f && (Hunger > 0.7f || Thirst > 0.7f))
    {
        ContextualMontage = EAnim_SurvivalMontageType::Exhausted;
    }

    if (ContextualMontage != EAnim_SurvivalMontageType::None)
    {
        return PlaySurvivalMontage(AnimInstance, ContextualMontage);
    }

    return false;
}

void USurvivalAnimMontageLibrary::InitializeMontageConfigs()
{
    UE_LOG(LogTemp, Log, TEXT("USurvivalAnimMontageLibrary::InitializeMontageConfigs - Initializing default montage configurations"));

    // Initialize default configurations for each montage type
    FAnim_MontageConfig GatheringConfig;
    GatheringConfig.PlayRate = 1.0f;
    GatheringConfig.BlendInTime = 0.3f;
    GatheringConfig.BlendOutTime = 0.3f;
    GatheringConfig.bLooping = false;
    GatheringConfig.bInterruptible = true;
    MontageConfigs.Add(EAnim_SurvivalMontageType::Gathering, GatheringConfig);

    FAnim_MontageConfig CraftingConfig;
    CraftingConfig.PlayRate = 0.8f;
    CraftingConfig.BlendInTime = 0.5f;
    CraftingConfig.BlendOutTime = 0.5f;
    CraftingConfig.bLooping = true;
    CraftingConfig.bInterruptible = true;
    MontageConfigs.Add(EAnim_SurvivalMontageType::Crafting, CraftingConfig);

    FAnim_MontageConfig DrinkingConfig;
    DrinkingConfig.PlayRate = 1.2f;
    DrinkingConfig.BlendInTime = 0.2f;
    DrinkingConfig.BlendOutTime = 0.3f;
    DrinkingConfig.bLooping = false;
    DrinkingConfig.bInterruptible = false;
    MontageConfigs.Add(EAnim_SurvivalMontageType::Drinking, DrinkingConfig);

    FAnim_MontageConfig EatingConfig;
    EatingConfig.PlayRate = 1.0f;
    EatingConfig.BlendInTime = 0.2f;
    EatingConfig.BlendOutTime = 0.3f;
    EatingConfig.bLooping = false;
    EatingConfig.bInterruptible = false;
    MontageConfigs.Add(EAnim_SurvivalMontageType::Eating, EatingConfig);

    FAnim_MontageConfig HidingConfig;
    HidingConfig.PlayRate = 0.6f;
    HidingConfig.BlendInTime = 0.8f;
    HidingConfig.BlendOutTime = 0.8f;
    HidingConfig.bLooping = true;
    HidingConfig.bInterruptible = true;
    MontageConfigs.Add(EAnim_SurvivalMontageType::Hiding, HidingConfig);

    FAnim_MontageConfig ClimbingConfig;
    ClimbingConfig.PlayRate = 0.9f;
    ClimbingConfig.BlendInTime = 0.4f;
    ClimbingConfig.BlendOutTime = 0.4f;
    ClimbingConfig.bLooping = false;
    ClimbingConfig.bInterruptible = true;
    MontageConfigs.Add(EAnim_SurvivalMontageType::Climbing, ClimbingConfig);

    FAnim_MontageConfig SwimmingConfig;
    SwimmingConfig.PlayRate = 1.1f;
    SwimmingConfig.BlendInTime = 0.6f;
    SwimmingConfig.BlendOutTime = 0.6f;
    SwimmingConfig.bLooping = true;
    SwimmingConfig.bInterruptible = true;
    MontageConfigs.Add(EAnim_SurvivalMontageType::Swimming, SwimmingConfig);

    FAnim_MontageConfig InjuredConfig;
    InjuredConfig.PlayRate = 0.7f;
    InjuredConfig.BlendInTime = 0.5f;
    InjuredConfig.BlendOutTime = 0.5f;
    InjuredConfig.bLooping = true;
    InjuredConfig.bInterruptible = true;
    MontageConfigs.Add(EAnim_SurvivalMontageType::Injured, InjuredConfig);

    FAnim_MontageConfig ExhaustedConfig;
    ExhaustedConfig.PlayRate = 0.5f;
    ExhaustedConfig.BlendInTime = 1.0f;
    ExhaustedConfig.BlendOutTime = 1.0f;
    ExhaustedConfig.bLooping = true;
    ExhaustedConfig.bInterruptible = true;
    MontageConfigs.Add(EAnim_SurvivalMontageType::Exhausted, ExhaustedConfig);

    FAnim_MontageConfig FearfulConfig;
    FearfulConfig.PlayRate = 1.3f;
    FearfulConfig.BlendInTime = 0.1f;
    FearfulConfig.BlendOutTime = 0.3f;
    FearfulConfig.bLooping = false;
    FearfulConfig.bInterruptible = false;
    MontageConfigs.Add(EAnim_SurvivalMontageType::Fearful, FearfulConfig);

    UE_LOG(LogTemp, Log, TEXT("USurvivalAnimMontageLibrary::InitializeMontageConfigs - Initialized %d montage configurations"), MontageConfigs.Num());
}

FString USurvivalAnimMontageLibrary::GetMontageAssetPath(EAnim_SurvivalMontageType MontageType)
{
    switch (MontageType)
    {
        case EAnim_SurvivalMontageType::Gathering:
            return TEXT("/Game/TranspersonalGame/Animation/Montages/AM_Gathering");
        case EAnim_SurvivalMontageType::Crafting:
            return TEXT("/Game/TranspersonalGame/Animation/Montages/AM_Crafting");
        case EAnim_SurvivalMontageType::Drinking:
            return TEXT("/Game/TranspersonalGame/Animation/Montages/AM_Drinking");
        case EAnim_SurvivalMontageType::Eating:
            return TEXT("/Game/TranspersonalGame/Animation/Montages/AM_Eating");
        case EAnim_SurvivalMontageType::Hiding:
            return TEXT("/Game/TranspersonalGame/Animation/Montages/AM_Hiding");
        case EAnim_SurvivalMontageType::Climbing:
            return TEXT("/Game/TranspersonalGame/Animation/Montages/AM_Climbing");
        case EAnim_SurvivalMontageType::Swimming:
            return TEXT("/Game/TranspersonalGame/Animation/Montages/AM_Swimming");
        case EAnim_SurvivalMontageType::Injured:
            return TEXT("/Game/TranspersonalGame/Animation/Montages/AM_Injured");
        case EAnim_SurvivalMontageType::Exhausted:
            return TEXT("/Game/TranspersonalGame/Animation/Montages/AM_Exhausted");
        case EAnim_SurvivalMontageType::Fearful:
            return TEXT("/Game/TranspersonalGame/Animation/Montages/AM_Fearful");
        default:
            return TEXT("");
    }
}