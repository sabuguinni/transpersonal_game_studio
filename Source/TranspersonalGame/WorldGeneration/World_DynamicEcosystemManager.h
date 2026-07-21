#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "World_DynamicEcosystemManager.generated.h"

// Forward declarations
class UWorld_BiomeManager;
class UWorld_ResourceDistributionManager;

UENUM(BlueprintType)
enum class EWorld_EcosystemZoneType : uint8
{
    Forest_Herbivore     UMETA(DisplayName = "Forest Herbivore Zone"),
    Forest_Predator      UMETA(DisplayName = "Forest Predator Zone"),
    Plains_Herd          UMETA(DisplayName = "Plains Herd Zone"),
    Plains_Apex          UMETA(DisplayName = "Plains Apex Zone"),
    Volcanic_Survivor    UMETA(DisplayName = "Volcanic Survivor Zone"),
    River_Amphibian      UMETA(DisplayName = "River Amphibian Zone"),
    Migration_Path       UMETA(DisplayName = "Migration Path"),
    Water_Source         UMETA(DisplayName = "Water Source Hub"),
    Nesting_Area         UMETA(DisplayName = "Nesting Area"),
    Feeding_Ground       UMETA(DisplayName = "Feeding Ground")
};

UENUM(BlueprintType)
enum class EWorld_WildlifeSpecies : uint8
{
    Triceratops          UMETA(DisplayName = "Triceratops"),
    Parasaurolophus      UMETA(DisplayName = "Parasaurolophus"),
    Carnotaurus          UMETA(DisplayName = "Carnotaurus"),
    Utahraptor           UMETA(DisplayName = "Utahraptor"),
    Brachiosaurus        UMETA(DisplayName = "Brachiosaurus"),
    Edmontosaurus        UMETA(DisplayName = "Edmontosaurus"),
    Ankylosaurus         UMETA(DisplayName = "Ankylosaurus"),
    TRex                 UMETA(DisplayName = "Tyrannosaurus Rex"),
    Allosaurus           UMETA(DisplayName = "Allosaurus"),
    Therizinosaurus      UMETA(DisplayName = "Therizinosaurus"),
    Ceratosaurus         UMETA(DisplayName = "Ceratosaurus"),
    Spinosaurus          UMETA(DisplayName = "Spinosaurus"),
    Baryonyx             UMETA(DisplayName = "Baryonyx")
};

USTRUCT(BlueprintType)
struct FWorld_EcosystemZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    EWorld_EcosystemZoneType ZoneType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    int32 MaxWildlifeCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float ActivityLevel;

    FWorld_EcosystemZone()
    {
        ZoneType = EWorld_EcosystemZoneType::Forest_Herbivore;
        Location = FVector::ZeroVector;
        Radius = 500.0f;
        BiomeType = EBiomeType::Forest;
        MaxWildlifeCount = 5;
        ActivityLevel = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FWorld_WildlifeSpawn
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wildlife")
    EWorld_WildlifeSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wildlife")
    FVector SpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wildlife")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wildlife")
    EWorld_EcosystemZoneType PreferredZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wildlife")
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wildlife")
    bool bIsApexPredator;

    FWorld_WildlifeSpawn()
    {
        Species = EWorld_WildlifeSpecies::Triceratops;
        SpawnLocation = FVector::ZeroVector;
        Scale = FVector(1.0f);
        PreferredZone = EWorld_EcosystemZoneType::Forest_Herbivore;
        TerritoryRadius = 200.0f;
        bIsApexPredator = false;
    }
};

USTRUCT(BlueprintType)
struct FWorld_EnvironmentalStoryElement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    FString ElementName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    FString StoryContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    EBiomeType AssociatedBiome;

    FWorld_EnvironmentalStoryElement()
    {
        ElementName = TEXT("Story_Element");
        Location = FVector::ZeroVector;
        Scale = FVector(1.0f);
        StoryContext = TEXT("Environmental narrative element");
        AssociatedBiome = EBiomeType::Forest;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API AWorld_DynamicEcosystemManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_DynamicEcosystemManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* RootMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* EcosystemInfluenceRadius;

public:
    // Ecosystem zone management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem Zones")
    TArray<FWorld_EcosystemZone> EcosystemZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wildlife")
    TArray<FWorld_WildlifeSpawn> WildlifeSpawns;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    TArray<FWorld_EnvironmentalStoryElement> StoryElements;

    // Dynamic ecosystem parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem Dynamics")
    float EcosystemUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem Dynamics")
    float WildlifeMigrationSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem Dynamics")
    float ResourceRegenerationRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem Dynamics")
    float PredatorPreyBalance;

    // Ecosystem zone functions
    UFUNCTION(BlueprintCallable, Category = "Ecosystem Management")
    void CreateEcosystemZone(EWorld_EcosystemZoneType ZoneType, FVector Location, float Radius, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Wildlife Management")
    void SpawnWildlifeInZone(EWorld_EcosystemZoneType ZoneType, EWorld_WildlifeSpecies Species, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void CreateStoryElement(const FString& ElementName, FVector Location, const FString& Context, EBiomeType Biome);

    // Ecosystem interaction functions
    UFUNCTION(BlueprintCallable, Category = "Ecosystem Dynamics")
    void UpdateEcosystemDynamics();

    UFUNCTION(BlueprintCallable, Category = "Ecosystem Dynamics")
    void TriggerMigrationEvent(EWorld_EcosystemZoneType FromZone, EWorld_EcosystemZoneType ToZone);

    UFUNCTION(BlueprintCallable, Category = "Ecosystem Dynamics")
    void SimulatePredatorHunt(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Ecosystem Dynamics")
    void UpdateResourceDistribution();

    // Ecosystem query functions
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ecosystem Queries")
    FWorld_EcosystemZone GetNearestEcosystemZone(FVector Location) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ecosystem Queries")
    TArray<FWorld_WildlifeSpawn> GetWildlifeInRadius(FVector Location, float Radius) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ecosystem Queries")
    float GetEcosystemDensity(FVector Location, float Radius) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ecosystem Queries")
    bool IsLocationSafeForPlayer(FVector Location, float SafetyRadius) const;

private:
    // Internal ecosystem management
    void InitializeDefaultEcosystem();
    void ValidateEcosystemBalance();
    void ProcessWildlifeInteractions();
    void UpdateEnvironmentalStorytellingElements();

    // Ecosystem state tracking
    UPROPERTY()
    float LastEcosystemUpdate;

    UPROPERTY()
    TArray<AActor*> SpawnedWildlifeActors;

    UPROPERTY()
    TArray<AActor*> StoryElementActors;

    UPROPERTY()
    TArray<AActor*> EcosystemZoneActors;
};