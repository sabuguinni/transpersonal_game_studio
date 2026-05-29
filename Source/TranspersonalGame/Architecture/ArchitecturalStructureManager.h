#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "../SharedTypes.h"
#include "ArchitecturalStructureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    None            UMETA(DisplayName = "None"),
    StonePillar     UMETA(DisplayName = "Stone Pillar"),
    Platform        UMETA(DisplayName = "Platform"),
    Wall            UMETA(DisplayName = "Wall"),
    Foundation      UMETA(DisplayName = "Foundation"),
    Ruins           UMETA(DisplayName = "Ruins")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EBiomeType BiomeType = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatheringLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsRuined = false;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::None;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        BiomeType = EBiomeType::Savana;
        WeatheringLevel = 0.5f;
        bIsRuined = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_StructureActor : public AActor
{
    GENERATED_BODY()

public:
    AArch_StructureActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StructureMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Data")
    FArch_StructureData StructureData;

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void InitializeStructure(const FArch_StructureData& InStructureData);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void ApplyWeathering(float WeatheringAmount);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void SetStructureType(EArch_StructureType NewType);

private:
    void UpdateMeshBasedOnType();
    void ApplyBiomeSpecificMaterials();
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UArchitecturalStructureManager : public UObject
{
    GENERATED_BODY()

public:
    UArchitecturalStructureManager();

    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor)
    void SpawnStructuresInBiome(EBiomeType BiomeType, int32 StructureCount = 50);

    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor)
    void SpawnStructureAtLocation(EArch_StructureType StructureType, FVector Location, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<AArch_StructureActor*> GetStructuresInRadius(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ClearAllStructures();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    int32 GetTotalStructureCount() const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Structures")
    TArray<AArch_StructureActor*> SpawnedStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TMap<EBiomeType, FVector> BiomeCoordinates;

private:
    void InitializeBiomeCoordinates();
    FVector GetRandomLocationInBiome(EBiomeType BiomeType, float Radius = 5000.0f);
    EArch_StructureType GetRandomStructureTypeForBiome(EBiomeType BiomeType);
};