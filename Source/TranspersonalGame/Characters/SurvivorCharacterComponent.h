#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivorCharacterComponent.generated.h"

// ============================================================
// Survivor Character Visual & Stats Component
// Agent #09 — Character Artist
// Cycle: PROD_CYCLE_AUTO_20260702_001
// ============================================================

UENUM(BlueprintType)
enum class EChar_SurvivorGender : uint8
{
    Male    UMETA(DisplayName = "Male"),
    Female  UMETA(DisplayName = "Female")
};

UENUM(BlueprintType)
enum class EChar_SurvivorBuild : uint8
{
    Lean    UMETA(DisplayName = "Lean"),
    Muscular UMETA(DisplayName = "Muscular"),
    Stocky  UMETA(DisplayName = "Stocky")
};

UENUM(BlueprintType)
enum class EChar_ArmorTier : uint8
{
    None        UMETA(DisplayName = "None"),
    Primitive   UMETA(DisplayName = "Primitive Leather"),
    Bone        UMETA(DisplayName = "Bone Reinforced"),
    DinoScale   UMETA(DisplayName = "Dinosaur Scale")
};

USTRUCT(BlueprintType)
struct FChar_SurvivorAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance")
    EChar_SurvivorGender Gender = EChar_SurvivorGender::Male;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance")
    EChar_SurvivorBuild Build = EChar_SurvivorBuild::Lean;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance")
    EChar_ArmorTier ArmorTier = EChar_ArmorTier::Primitive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance")
    FLinearColor SkinTone = FLinearColor(0.6f, 0.4f, 0.25f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance")
    bool bHasBoneNecklace = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance")
    bool bHasFeatherHeaddress = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance")
    float WoundSeverity = 0.0f; // 0=clean, 1=heavily wounded (drives material blend)
};

USTRUCT(BlueprintType)
struct FChar_SurvivorSurvivalStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Survival")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Survival")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Survival")
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Survival")
    float Thirst = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Survival")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Survival")
    float Fear = 0.0f; // 0=calm, 100=panicking

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Survival")
    float Temperature = 37.0f; // Body temp in Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Survival")
    float StealthLevel = 0.5f; // 0=fully visible, 1=fully hidden
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API USurvivorCharacterComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivorCharacterComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ---- Appearance ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance")
    FChar_SurvivorAppearance Appearance;

    // ---- Survival Stats ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Survival")
    FChar_SurvivorSurvivalStats SurvivalStats;

    // ---- Mesh References ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Mesh")
    TSoftObjectPtr<USkeletalMesh> MaleSurvivorMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Mesh")
    TSoftObjectPtr<USkeletalMesh> FemaleSurvivorMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Mesh")
    TSoftObjectPtr<USkeletalMesh> CurrentActiveMesh;

    // ---- Camera ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Camera")
    float CameraBoomLength = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Camera")
    FVector CameraBoomOffset = FVector(0.0f, 0.0f, 60.0f);

    // ---- Functions ----
    UFUNCTION(BlueprintCallable, Category = "Character|Survival")
    void ApplyDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Character|Survival")
    void ConsumeFood(float NutritionValue);

    UFUNCTION(BlueprintCallable, Category = "Character|Survival")
    void ConsumeWater(float HydrationValue);

    UFUNCTION(BlueprintCallable, Category = "Character|Survival")
    void SetFearLevel(float NewFear);

    UFUNCTION(BlueprintCallable, Category = "Character|Appearance")
    void UpgradeArmor(EChar_ArmorTier NewTier);

    UFUNCTION(BlueprintCallable, Category = "Character|Appearance")
    void ApplyWoundVisual(float Severity);

    UFUNCTION(BlueprintPure, Category = "Character|Survival")
    bool IsAlive() const;

    UFUNCTION(BlueprintPure, Category = "Character|Survival")
    bool IsExhausted() const;

    UFUNCTION(BlueprintPure, Category = "Character|Survival")
    bool IsPanicking() const;

    UFUNCTION(BlueprintCallable, Category = "Character|Survival")
    void TickSurvivalDecay(float DeltaTime);

private:
    // Decay rates per second
    static constexpr float HungerDecayRate  = 0.5f;
    static constexpr float ThirstDecayRate  = 0.8f;
    static constexpr float StaminaRecovery  = 5.0f;
    static constexpr float FearDecayRate    = 2.0f;

    float TimeSinceLastDecayTick = 0.0f;
};
