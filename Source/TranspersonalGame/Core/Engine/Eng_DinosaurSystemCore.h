#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "Eng_DinosaurSystemCore.generated.h"

// Forward declarations
class USkeletalMesh;
class UStaticMesh;
class UAnimBlueprint;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_DinosaurSpecies
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    FString SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    EEng_DinosaurType DinosaurType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    FString MeshPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    FString AnimBlueprintPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float BaseHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    EEng_BiomeType PreferredBiome;

    FEng_DinosaurSpecies()
    {
        SpeciesName = TEXT("Unknown");
        DinosaurType = EEng_DinosaurType::Carnivore;
        MeshPath = TEXT("");
        AnimBlueprintPath = TEXT("");
        BaseHealth = 100.0f;
        MovementSpeed = 300.0f;
        AttackDamage = 25.0f;
        DetectionRange = 1000.0f;
        PreferredBiome = EEng_BiomeType::Savanna;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_DinosaurSpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FEng_DinosaurSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FVector SpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FRotator SpawnRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FString ActorLabel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    bool bIsActive;

    FEng_DinosaurSpawnData()
    {
        SpawnLocation = FVector::ZeroVector;
        SpawnRotation = FRotator::ZeroRotator;
        ActorLabel = TEXT("Dinosaur");
        bIsActive = true;
    }
};

/**
 * Engine Architect Dinosaur System Core
 * Manages dinosaur species definitions, spawning, and integration with biome system
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_DinosaurSystemCore : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_DinosaurSystemCore();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Species management
    UFUNCTION(BlueprintCallable, Category = "Dinosaur System")
    void InitializeSpeciesDatabase();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur System")
    FEng_DinosaurSpecies GetSpeciesByType(EEng_DinosaurType DinosaurType);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur System")
    TArray<FEng_DinosaurSpecies> GetSpeciesForBiome(EEng_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur System")
    bool RegisterSpecies(const FEng_DinosaurSpecies& NewSpecies);

    // Spawning system
    UFUNCTION(BlueprintCallable, Category = "Dinosaur System")
    AActor* SpawnDinosaur(const FEng_DinosaurSpawnData& SpawnData);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur System")
    TArray<AActor*> SpawnDinosaursInBiome(EEng_BiomeType BiomeType, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur System")
    void SpawnDefaultDinosaurs();

    // Asset management
    UFUNCTION(BlueprintCallable, Category = "Dinosaur System")
    USkeletalMesh* LoadDinosaurMesh(const FString& MeshPath);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur System")
    UAnimBlueprint* LoadDinosaurAnimBlueprint(const FString& AnimPath);

    // Validation and testing
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Dinosaur System")
    void ValidateSpeciesDatabase();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Dinosaur System")
    void TestDinosaurSpawning();

    // Debug functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur System")
    void LogSpeciesDatabase();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur System")
    int32 GetActiveSpeciesCount() const;

protected:
    // Species database
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Species Database", meta = (AllowPrivateAccess = "true"))
    TArray<FEng_DinosaurSpecies> SpeciesDatabase;

    // Spawned dinosaurs tracking
    UPROPERTY(BlueprintReadOnly, Category = "Spawning", meta = (AllowPrivateAccess = "true"))
    TArray<AActor*> SpawnedDinosaurs;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    bool bAutoInitializeOnStartup;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    bool bEnableDebugLogging;

private:
    // Internal helpers
    void SetupDefaultSpecies();
    FVector GetBiomeSpawnLocation(EEng_BiomeType BiomeType);
    bool ValidateSpecies(const FEng_DinosaurSpecies& Species);
    void CleanupInvalidDinosaurs();
};