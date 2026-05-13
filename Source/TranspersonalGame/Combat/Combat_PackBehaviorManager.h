#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "Combat_PackBehaviorManager.generated.h"

// Pack formation types for tactical positioning
UENUM(BlueprintType)
enum class ECombat_PackFormation : uint8
{
    None            UMETA(DisplayName = "No Formation"),
    Circle          UMETA(DisplayName = "Circle Formation"),
    Line            UMETA(DisplayName = "Line Formation"),
    Wedge           UMETA(DisplayName = "Wedge Formation"),
    Ambush          UMETA(DisplayName = "Ambush Formation"),
    Scatter         UMETA(DisplayName = "Scatter Formation")
};

// Pack role for individual members
UENUM(BlueprintType)
enum class ECombat_PackRole : uint8
{
    Alpha           UMETA(DisplayName = "Alpha Leader"),
    Beta            UMETA(DisplayName = "Beta Lieutenant"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Scout           UMETA(DisplayName = "Scout"),
    Defender        UMETA(DisplayName = "Defender"),
    Juvenile        UMETA(DisplayName = "Juvenile")
};

// Pack member data
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_PackMember
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Pack Member")
    TWeakObjectPtr<AActor> Actor;

    UPROPERTY(BlueprintReadOnly, Category = "Pack Member")
    ECombat_PackRole Role;

    UPROPERTY(BlueprintReadOnly, Category = "Pack Member")
    float HealthPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Pack Member")
    float StaminaPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Pack Member")
    FVector LastKnownPosition;

    UPROPERTY(BlueprintReadOnly, Category = "Pack Member")
    float LastUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "Pack Member")
    bool bIsInCombat;

    UPROPERTY(BlueprintReadOnly, Category = "Pack Member")
    TWeakObjectPtr<AActor> CurrentTarget;

    FCombat_PackMember()
    {
        Role = ECombat_PackRole::Hunter;
        HealthPercentage = 1.0f;
        StaminaPercentage = 1.0f;
        LastKnownPosition = FVector::ZeroVector;
        LastUpdateTime = 0.0f;
        bIsInCombat = false;
    }
};

// Pack data structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_PackData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    FString PackID;

    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    TArray<FCombat_PackMember> Members;

    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    TWeakObjectPtr<AActor> AlphaLeader;

    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    ECombat_PackFormation CurrentFormation;

    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    FVector FormationCenter;

    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    float FormationRadius;

    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    TWeakObjectPtr<AActor> PrimaryTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    float LastFormationUpdate;

    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    bool bIsHunting;

    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    bool bIsDefending;

    FCombat_PackData()
    {
        CurrentFormation = ECombat_PackFormation::None;
        FormationCenter = FVector::ZeroVector;
        FormationRadius = 500.0f;
        LastFormationUpdate = 0.0f;
        bIsHunting = false;
        bIsDefending = false;
    }
};

/**
 * Pack Behavior Manager - Handles coordinated group combat AI
 * Manages pack formations, role assignments, and tactical coordination
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombat_PackBehaviorManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCombat_PackBehaviorManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

    // Pack management
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    FString CreatePack(const TArray<AActor*>& PackMembers, AActor* AlphaLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    bool AddMemberToPack(const FString& PackID, AActor* NewMember, ECombat_PackRole Role);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    bool RemoveMemberFromPack(const FString& PackID, AActor* Member);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void DisbandPack(const FString& PackID);

    // Formation control
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    bool SetPackFormation(const FString& PackID, ECombat_PackFormation Formation, const FVector& Center);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    FVector GetFormationPositionForMember(const FString& PackID, AActor* Member);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    bool IsPackInFormation(const FString& PackID, float ToleranceRadius = 200.0f);

    // Combat coordination
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    bool SetPackTarget(const FString& PackID, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    AActor* GetPackTarget(const FString& PackID);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    TArray<AActor*> GetPackMembers(const FString& PackID);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    AActor* GetPackAlpha(const FString& PackID);

    // Pack queries
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    FString FindPackForActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    ECombat_PackRole GetActorPackRole(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    float GetPackCohesion(const FString& PackID);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    bool IsPackHealthy(const FString& PackID, float HealthThreshold = 0.5f);

    // Tactical decisions
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    ECombat_PackFormation GetOptimalFormation(const FString& PackID, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    bool ShouldPackRetreat(const FString& PackID);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    AActor* GetBestPackTarget(const FString& PackID, const TArray<AActor*>& PotentialTargets);

protected:
    // Pack data storage
    UPROPERTY()
    TMap<FString, FCombat_PackData> ActivePacks;

    // Update intervals
    UPROPERTY(EditAnywhere, Category = "Pack Behavior")
    float PackUpdateInterval;

    UPROPERTY(EditAnywhere, Category = "Pack Behavior")
    float FormationUpdateInterval;

    // Formation parameters
    UPROPERTY(EditAnywhere, Category = "Pack Behavior")
    float DefaultFormationRadius;

    UPROPERTY(EditAnywhere, Category = "Pack Behavior")
    float FormationTolerance;

    // Combat parameters
    UPROPERTY(EditAnywhere, Category = "Pack Behavior")
    float PackCohesionRadius;

    UPROPERTY(EditAnywhere, Category = "Pack Behavior")
    float RetreatHealthThreshold;

    UPROPERTY(EditAnywhere, Category = "Pack Behavior")
    float MaxPackSize;

    // Internal methods
    void UpdatePackStates(float DeltaTime);
    void UpdatePackFormations(float DeltaTime);
    void UpdatePackCombat(float DeltaTime);
    void CleanupInvalidPacks();

    FVector CalculateCircleFormationPosition(const FCombat_PackData& Pack, int32 MemberIndex);
    FVector CalculateLineFormationPosition(const FCombat_PackData& Pack, int32 MemberIndex);
    FVector CalculateWedgeFormationPosition(const FCombat_PackData& Pack, int32 MemberIndex);
    FVector CalculateAmbushFormationPosition(const FCombat_PackData& Pack, int32 MemberIndex);

    bool IsValidPackMember(AActor* Actor);
    void UpdateMemberStatus(FCombat_PackMember& Member);
    float CalculatePackThreatLevel(const FCombat_PackData& Pack, AActor* Target);

private:
    float LastPackUpdate;
    float LastFormationUpdate;
    int32 NextPackID;
};