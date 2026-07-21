#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "WorldArchitectureManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ArchitecturalElement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture")
    FString ElementName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture")
    EBiomeType AssociatedBiome = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture")
    bool bIsInteractable = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture")
    float StructuralIntegrity = 100.0f;

    FEng_ArchitecturalElement()
    {
        ElementName = TEXT("DefaultElement");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorldArchitectureManager : public AActor
{
    GENERATED_BODY()

public:
    AWorldArchitectureManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void InitializeArchitecturalElements();

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void SpawnArchitecturalElement(const FEng_ArchitecturalElement& Element);

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void CreateCaveSystem(EBiomeType BiomeType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void CreateRockFormation(EBiomeType BiomeType, const FVector& Location, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void CreateTribalStructure(const FVector& Location, const FString& StructureType);

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    TArray<FEng_ArchitecturalElement> GetArchitecturalElementsInRadius(const FVector& Center, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    bool ValidateArchitecturalPlacement(const FVector& Location, float MinDistance = 1000.0f) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World Architecture")
    void GenerateDefaultArchitecture();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(BlueprintReadOnly, Category = "World Architecture")
    TArray<FEng_ArchitecturalElement> ArchitecturalElements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Architecture")
    float MinElementSpacing = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Architecture")
    int32 MaxElementsPerBiome = 50;

    void CreateBiomeSpecificArchitecture(EBiomeType BiomeType);
    FVector GetValidSpawnLocation(EBiomeType BiomeType) const;
    void SpawnRockFormationMesh(const FVector& Location, const FRotator& Rotation, const FVector& Scale);
};

#include "WorldArchitectureManager.generated.h"