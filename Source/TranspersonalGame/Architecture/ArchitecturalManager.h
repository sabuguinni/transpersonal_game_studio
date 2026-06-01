#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "ArchitecturalManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    None = 0,
    Pillar,
    Wall,
    Archway,
    Platform,
    Stairs,
    Watchtower,
    Ruins
};

USTRUCT(BlueprintType)
struct FArch_StructureData
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
    float DegradationLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasVegetationOvergrowth = false;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::None;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        DegradationLevel = 0.0f;
        bHasVegetationOvergrowth = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArchitecturalManager : public AActor
{
    GENERATED_BODY()

public:
    AArchitecturalManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FArch_StructureData> StructureDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    int32 MaxStructuresPerBiome = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float MinDistanceBetweenStructures = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TMap<EBiomeType, int32> StructureCountPerBiome;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SpawnStructureAtLocation(EArch_StructureType StructureType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SpawnStructuresInBiome(EBiomeType BiomeType, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ClearAllStructures();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    int32 GetStructureCount() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FArch_StructureData> GetStructuresInRadius(FVector Center, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeatheringToStructures(float WeatheringAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateStructureDegradation(float DeltaTime);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void GenerateArchitecturalLayout();

protected:
    UFUNCTION()
    void ValidateStructurePlacement();

    UFUNCTION()
    FVector GetRandomLocationInBiome(EBiomeType BiomeType) const;

    UFUNCTION()
    bool IsLocationValid(FVector Location) const;

    UFUNCTION()
    AActor* SpawnStructureMesh(EArch_StructureType StructureType, FVector Location, FRotator Rotation);
};