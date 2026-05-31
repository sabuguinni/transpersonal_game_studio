#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Arch_StructuralManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    StonePillar     UMETA(DisplayName = "Stone Pillar"),
    StoneWall       UMETA(DisplayName = "Stone Wall"),
    StoneArch       UMETA(DisplayName = "Stone Arch"),
    Ruins           UMETA(DisplayName = "Ancient Ruins"),
    ShelterEntrance UMETA(DisplayName = "Shelter Entrance"),
    CaveOpening     UMETA(DisplayName = "Cave Opening")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::StonePillar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatheringLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasMossOvergrowth = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasCarvedSymbols = false;

    FArch_StructureConfig()
    {
        StructureType = EArch_StructureType::StonePillar;
        Scale = FVector(1.0f, 1.0f, 1.0f);
        WeatheringLevel = 0.5f;
        bHasMossOvergrowth = true;
        bHasCarvedSymbols = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_StructuralManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_StructuralManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StructureMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FArch_StructureConfig StructureConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<UMaterialInterface*> WeatheredMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<UMaterialInterface*> MossyMaterials;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetStructureType(EArch_StructureType NewType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeathering(float WeatheringAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ToggleMossOvergrowth(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void AddCarvedSymbols(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor = true)
    void GenerateRandomStructure();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FVector GetStructureBounds() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsStructureStable() const;

protected:
    UFUNCTION()
    void UpdateStructureAppearance();

    UFUNCTION()
    void ApplyMaterialVariations();

private:
    float LastWeatheringUpdate;
    bool bIsInitialized;
};