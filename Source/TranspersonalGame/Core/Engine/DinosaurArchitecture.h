#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "AIController.h"
#include "../../SharedTypes.h"
#include "DinosaurArchitecture.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float DetectionRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Hunger = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Aggression = 0.5f;

    FEng_DinosaurStats()
    {
        Health = MaxHealth = 100.0f;
        AttackDamage = 25.0f;
        MovementSpeed = 300.0f;
        DetectionRadius = 1000.0f;
        AttackRange = 200.0f;
        Hunger = 50.0f;
        Aggression = 0.5f;
    }
};

UENUM(BlueprintType)
enum class EEng_DinosaurBehaviorState : uint8
{
    Idle         UMETA(DisplayName = "Idle"),
    Wandering    UMETA(DisplayName = "Wandering"),
    Hunting      UMETA(DisplayName = "Hunting"),
    Fleeing      UMETA(DisplayName = "Fleeing"),
    Feeding      UMETA(DisplayName = "Feeding"),
    Attacking    UMETA(DisplayName = "Attacking"),
    Sleeping     UMETA(DisplayName = "Sleeping"),
    Territorial  UMETA(DisplayName = "Territorial")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_DinosaurSpeciesData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    FString SpeciesName = "Unknown";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    EDinosaurType DinosaurType = EDinosaurType::Carnivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    EBiomeType PreferredBiome = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    FEng_DinosaurStats BaseStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    FString MeshAssetPath = "/Game/Dinosaur_Pack/Trex/Mesh/SKM_Trex_Skin";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    int32 PackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float TerritoryRadius = 2000.0f;

    FEng_DinosaurSpeciesData()
    {
        SpeciesName = "Unknown";
        DinosaurType = EDinosaurType::Carnivore;
        PreferredBiome = EBiomeType::Savana;
        MeshAssetPath = "/Game/Dinosaur_Pack/Trex/Mesh/SKM_Trex_Skin";
        bIsPackHunter = false;
        PackSize = 1;
        TerritoryRadius = 2000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_DinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    AEng_DinosaurBase();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    FEng_DinosaurSpeciesData SpeciesData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    FEng_DinosaurStats CurrentStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    EEng_DinosaurBehaviorState CurrentBehaviorState = EEng_DinosaurBehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    TArray<AEng_DinosaurBase*> PackMembers;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void InitializeDinosaur(const FEng_DinosaurSpeciesData& InSpeciesData);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void SetBehaviorState(EEng_DinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void TakeDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void AttackTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    bool IsInAttackRange(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void StartHunting(AActor* Prey);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void ReturnToTerritory();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void JoinPack(AEng_DinosaurBase* PackLeader);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION()
    void UpdateBehavior(float DeltaTime);

    UFUNCTION()
    void UpdateStats(float DeltaTime);

    UFUNCTION()
    void ScanForTargets();

private:
    float LastBehaviorUpdate = 0.0f;
    float BehaviorUpdateInterval = 1.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_DinosaurSpeciesManager : public UObject
{
    GENERATED_BODY()

public:
    UEng_DinosaurSpeciesManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    TArray<FEng_DinosaurSpeciesData> SpeciesDatabase;

    UFUNCTION(BlueprintCallable, Category = "Species")
    void InitializeSpeciesDatabase();

    UFUNCTION(BlueprintCallable, Category = "Species")
    FEng_DinosaurSpeciesData GetSpeciesData(const FString& SpeciesName) const;

    UFUNCTION(BlueprintCallable, Category = "Species")
    TArray<FString> GetSpeciesForBiome(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Species")
    AEng_DinosaurBase* SpawnDinosaurAtLocation(const FString& SpeciesName, const FVector& Location, UWorld* World);

private:
    void SetupTRexData();
    void SetupVelociraptorData();
    void SetupTriceratopsData();
    void SetupBrachiosaurusData();
    void SetupAnkylosaurusData();
};