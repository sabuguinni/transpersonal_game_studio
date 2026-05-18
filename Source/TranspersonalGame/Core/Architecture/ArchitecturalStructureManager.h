#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "../SharedTypes.h"
#include "ArchitecturalStructureManager.generated.h"

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
    FRotator SpawnRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructureScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsInteractable;

    FArch_StructureData()
    {
        StructureName = TEXT("DefaultStructure");
        BiomeType = EBiomeType::Savana;
        SpawnLocation = FVector::ZeroVector;
        SpawnRotation = FRotator::ZeroRotator;
        StructureScale = 1.0f;
        bIsInteractable = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArchitecturalStructureManager : public AActor
{
    GENERATED_BODY()

public:
    AArchitecturalStructureManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FArch_StructureData> StructureDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<AActor*> SpawnedStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float MinDistanceBetweenStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    int32 MaxStructuresPerBiome;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SpawnStructureAtLocation(const FArch_StructureData& StructureData);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SpawnStructuresForBiome(EBiomeType BiomeType, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ClearAllStructures();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<AActor*> GetStructuresInBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void GenerateDefaultStructures();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsLocationSuitableForStructure(const FVector& Location, float MinDistance);

protected:
    UFUNCTION()
    void InitializeStructureDatabase();

    UFUNCTION()
    AActor* CreateStructureActor(const FArch_StructureData& StructureData);

    UFUNCTION()
    FVector GetBiomeBaseLocation(EBiomeType BiomeType);
};