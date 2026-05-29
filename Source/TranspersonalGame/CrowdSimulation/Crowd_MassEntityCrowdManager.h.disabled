#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "TranspersonalGame/SharedTypes.h"
#include "Crowd_MassEntityCrowdManager.generated.h"

UENUM(BlueprintType)
enum class ECrowd_BiomeType : uint8
{
    Swamp       UMETA(DisplayName = "Pântano"),
    Forest      UMETA(DisplayName = "Floresta"),
    Savanna     UMETA(DisplayName = "Savana"),
    Desert      UMETA(DisplayName = "Deserto"),
    Mountain    UMETA(DisplayName = "Montanha")
};

UENUM(BlueprintType)
enum class ECrowd_SpeciesType : uint8
{
    LargeHerbivore  UMETA(DisplayName = "Herbívoro Grande"),
    SmallHerbivore  UMETA(DisplayName = "Herbívoro Pequeno"),
    Carnivore       UMETA(DisplayName = "Carnívoro"),
    Scavenger       UMETA(DisplayName = "Necrófago"),
    Aquatic         UMETA(DisplayName = "Aquático"),
    Amphibian       UMETA(DisplayName = "Anfíbio"),
    Flying          UMETA(DisplayName = "Voador")
};

UENUM(BlueprintType)
enum class ECrowd_GroupBehavior : uint8
{
    Grazing     UMETA(DisplayName = "Pastando"),
    Migrating   UMETA(DisplayName = "Migrando"),
    Fleeing     UMETA(DisplayName = "Fugindo"),
    Hunting     UMETA(DisplayName = "Caçando"),
    Resting     UMETA(DisplayName = "Descansando"),
    Drinking    UMETA(DisplayName = "Bebendo")
};

UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    High        UMETA(DisplayName = "Alto"),
    Medium      UMETA(DisplayName = "Médio"),
    Low         UMETA(DisplayName = "Baixo"),
    Culled      UMETA(DisplayName = "Removido")
};

USTRUCT(BlueprintType)
struct FCrowd_BiomeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector BiomeCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeRadius = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 MaxEntitiesPerBiome = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<ECrowd_SpeciesType> PreferredSpecies;

    FCrowd_BiomeSettings()
    {
        BiomeCenter = FVector::ZeroVector;
        BiomeRadius = 10000.0f;
        MaxEntitiesPerBiome = 5000;
    }
};

USTRUCT(BlueprintType)
struct FCrowd_GroupData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    int32 GroupID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    ECrowd_BiomeType BiomeType = ECrowd_BiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    ECrowd_SpeciesType SpeciesType = ECrowd_SpeciesType::SmallHerbivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    FVector GroupCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    int32 GroupSize = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    ECrowd_GroupBehavior CurrentBehavior = ECrowd_GroupBehavior::Grazing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    ECrowd_LODLevel CurrentLOD = ECrowd_LODLevel::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    FVector ThreatLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    float BehaviorTimer = 0.0f;

    FCrowd_GroupData()
    {
        GroupID = 0;
        BiomeType = ECrowd_BiomeType::Savanna;
        SpeciesType = ECrowd_SpeciesType::SmallHerbivore;
        GroupCenter = FVector::ZeroVector;
        GroupSize = 10;
        CurrentBehavior = ECrowd_GroupBehavior::Grazing;
        CurrentLOD = ECrowd_LODLevel::Medium;
        TargetLocation = FVector::ZeroVector;
        ThreatLocation = FVector::ZeroVector;
        BehaviorTimer = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassEntityCrowdManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassEntityCrowdManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Configuração do sistema
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    int32 MaxCrowdEntities = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float CrowdUpdateRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LODDistanceNear = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LODDistanceMedium = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LODDistanceFar = 8000.0f;

    // Dados dos biomas
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TMap<ECrowd_BiomeType, FCrowd_BiomeSettings> BiomeSettings;

    // Dados dos grupos de multidão
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd Data")
    TArray<FCrowd_GroupData> CrowdGroups;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd Data")
    int32 CurrentCrowdCount = 0;

    // Funções públicas
    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    int32 GetTotalCrowdCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    int32 GetActiveGroupCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    TArray<FCrowd_GroupData> GetCrowdGroups() const;

private:
    // Entidades activas
    TArray<int32> ActiveEntities;

    // Funções internas
    void InitializeBiomeSettings();
    void InitializeCrowdSystem();
    void SpawnInitialCrowds();
    void SpawnCrowdInBiome(ECrowd_BiomeType BiomeType, const FCrowd_BiomeSettings& Settings);
    FVector GenerateRandomLocationInBiome(const FCrowd_BiomeSettings& Settings);
    
    void UpdateCrowdSystem(float DeltaTime);
    void UpdateGroupMovement(FCrowd_GroupData& Group, float DeltaTime);
    void UpdateGroupBehavior(FCrowd_GroupData& Group, float DeltaTime);
    void ClampGroupToBiome(FCrowd_GroupData& Group);
    
    void UpdateCrowdLOD();
    void UpdateGroupBehaviors(float DeltaTime);
    void ProcessGroupInteraction(FCrowd_GroupData& GroupA, FCrowd_GroupData& GroupB);
    void ConsiderSpawningNewGroups();
};