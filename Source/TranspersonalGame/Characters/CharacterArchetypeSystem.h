// CharacterArchetypeSystem.h
// Character Archetype System for Transpersonal Game Studio
// Manages NPC archetypes and their visual/behavioral traits

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "Components/SkeletalMeshComponent.h"
#include "CharacterArchetypeSystem.generated.h"

UENUM(BlueprintType)
enum class ECharacterArchetype : uint8
{
    Protagonist     UMETA(DisplayName = "Protagonist"),
    TribalElder     UMETA(DisplayName = "Tribal Elder"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Gatherer        UMETA(DisplayName = "Gatherer"),
    Warrior         UMETA(DisplayName = "Warrior"),
    Child           UMETA(DisplayName = "Child"),
    Outcast         UMETA(DisplayName = "Outcast"),
    Shaman          UMETA(DisplayName = "Shaman"),
    Crafter         UMETA(DisplayName = "Crafter"),
    Scout           UMETA(DisplayName = "Scout")
};

UENUM(BlueprintType)
enum class ETribalAffiliation : uint8
{
    None            UMETA(DisplayName = "No Tribe"),
    ForestDwellers  UMETA(DisplayName = "Forest Dwellers"),
    PlainsNomads    UMETA(DisplayName = "Plains Nomads"),
    CaveDwellers    UMETA(DisplayName = "Cave Dwellers"),
    RiverPeople     UMETA(DisplayName = "River People"),
    MountainClans   UMETA(DisplayName = "Mountain Clans")
};

UENUM(BlueprintType)
enum class ECharacterAge : uint8
{
    Child           UMETA(DisplayName = "Child (8-16)"),
    YoungAdult      UMETA(DisplayName = "Young Adult (17-30)"),
    Adult           UMETA(DisplayName = "Adult (31-45)"),
    Elder           UMETA(DisplayName = "Elder (46+)")
};

USTRUCT(BlueprintType)
struct FCharacterPhysicalTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float Height = 170.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float Weight = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float MuscleDefinition = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float WeatheringLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float ScarLevel = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    FLinearColor SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    FLinearColor HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    FLinearColor EyeColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);
};

USTRUCT(BlueprintType)
struct FCharacterClothing
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<USkeletalMesh> TorsoMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<USkeletalMesh> LegsMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<USkeletalMesh> FeetMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<USkeletalMesh> AccessoriesMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    float WearLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    float DirtLevel = 0.4f;
};

USTRUCT(BlueprintType)
struct FCharacterArchetypeData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    ECharacterArchetype ArchetypeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    FString ArchetypeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    ETribalAffiliation DefaultTribe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    ECharacterAge AgeRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    FCharacterPhysicalTraits PhysicalTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FCharacterClothing DefaultClothing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<USkeletalMesh> MetaHumanBaseMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<UClass> MetaHumanBlueprint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AggressionLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float CuriosityLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FearLevel = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float SocialLevel = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills")
    TArray<FString> PrimarySkills;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills")
    TArray<FString> SecondarySkills;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    float SpawnWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    int32 MaxInstancesInScene = 10;
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCharacterArchetypeDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetypes")
    TArray<FCharacterArchetypeData> Archetypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float VariationRange = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxCharactersInScene = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistanceMultiplier = 1.0f;

    UFUNCTION(BlueprintCallable, Category = "Archetypes")
    FCharacterArchetypeData GetArchetypeData(ECharacterArchetype ArchetypeType) const;

    UFUNCTION(BlueprintCallable, Category = "Archetypes")
    TArray<FCharacterArchetypeData> GetArchetypesByTribe(ETribalAffiliation Tribe) const;

    UFUNCTION(BlueprintCallable, Category = "Archetypes")
    FCharacterArchetypeData GetRandomArchetype() const;

    UFUNCTION(BlueprintCallable, Category = "Archetypes")
    FCharacterPhysicalTraits GenerateVariedTraits(const FCharacterPhysicalTraits& BaseTraits) const;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACharacterArchetypeManager : public AActor
{
    GENERATED_BODY()

public:
    ACharacterArchetypeManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TSoftObjectPtr<UCharacterArchetypeDataAsset> ArchetypeDataAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveCharacters = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> ActiveCharacters;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TMap<ECharacterArchetype, int32> ArchetypeCount;

public:
    UFUNCTION(BlueprintCallable, Category = "Character Management")
    AActor* SpawnCharacterOfArchetype(ECharacterArchetype ArchetypeType, FVector Location, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    void DespawnCharacter(AActor* Character);

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    void UpdateCharacterLODs();

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    int32 GetArchetypeCount(ECharacterArchetype ArchetypeType) const;

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    bool CanSpawnArchetype(ECharacterArchetype ArchetypeType) const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnCharacterSpawned(AActor* Character, ECharacterArchetype ArchetypeType);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnCharacterDespawned(AActor* Character, ECharacterArchetype ArchetypeType);

private:
    FTimerHandle UpdateTimer;
    
    void PerformUpdate();
    void OptimizeCharacterPerformance();
    void UpdateArchetypeCounts();
};