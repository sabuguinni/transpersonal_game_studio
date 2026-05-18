#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "../SharedTypes.h"
#include "PrimitiveStructureManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FString StructureName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector SpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructureHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsHabitable;

    FArch_StructureData()
    {
        StructureName = TEXT("Unknown Structure");
        BiomeType = EBiomeType::Savana;
        SpawnLocation = FVector::ZeroVector;
        StructureHealth = 100.0f;
        bIsHabitable = false;
    }
};

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    None            UMETA(DisplayName = "None"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    StoneArchway    UMETA(DisplayName = "Stone Archway"),
    RockShelter     UMETA(DisplayName = "Rock Shelter"),
    StonePillar     UMETA(DisplayName = "Stone Pillar"),
    PrimitiveHut    UMETA(DisplayName = "Primitive Hut"),
    RitualCircle    UMETA(DisplayName = "Ritual Circle")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APrimitiveStructureManager : public AActor
{
    GENERATED_BODY()

public:
    APrimitiveStructureManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StructureMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteractionVolume;

    // Structure Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EArch_StructureType StructureType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FArch_StructureData StructureData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<UStaticMesh*> StructureMeshVariants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float InteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bCanProvideShel ter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float ShelterEffectiveness;

public:
    virtual void Tick(float DeltaTime) override;

    // Structure Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void InitializeStructure(EArch_StructureType Type, const FVector& Location, EBiomeType Biome);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetStructureMesh(UStaticMesh* NewMesh);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CanPlayerInteract(const FVector& PlayerLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void OnPlayerInteract();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    float GetShelterValue() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsStructureIntact() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void DamageStructure(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RepairStructure(float RepairAmount);

    // Biome Integration
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void AdaptToBiome(EBiomeType Biome);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FVector> GetSpawnPointsAroundStructure(int32 NumPoints) const;

protected:
    UFUNCTION()
    void OnInteractionVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    void UpdateStructureAppearance();
    void SetupInteractionVolume();
    UStaticMesh* SelectMeshForBiome(EBiomeType Biome) const;
};