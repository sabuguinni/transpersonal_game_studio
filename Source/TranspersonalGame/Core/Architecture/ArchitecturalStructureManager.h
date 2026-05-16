#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "SharedTypes.h"
#include "ArchitecturalStructureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    Archway         UMETA(DisplayName = "Stone Archway"),
    Pillar          UMETA(DisplayName = "Stone Pillar"),
    Wall            UMETA(DisplayName = "Stone Wall"),
    Foundation      UMETA(DisplayName = "Foundation"),
    Ruin            UMETA(DisplayName = "Ancient Ruin"),
    Chamber         UMETA(DisplayName = "Stone Chamber")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::Archway;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Dimensions = FVector(400.0f, 200.0f, 600.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatheringLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasMossGrowth = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsInteractable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FString HistoricalContext = TEXT("Ancient Cretaceous structure");

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::Archway;
        Dimensions = FVector(400.0f, 200.0f, 600.0f);
        WeatheringLevel = 0.5f;
        bHasMossGrowth = true;
        bIsInteractable = false;
        HistoricalContext = TEXT("Ancient Cretaceous structure");
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
    UStaticMeshComponent* StructureMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteractionVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Data")
    FArch_StructureData StructureData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Assets")
    TMap<EArch_StructureType, UStaticMesh*> StructureMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* WeatheredStoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* MossyMaterial;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetStructureType(EArch_StructureType NewType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeathering(float WeatheringAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetMossGrowth(bool bEnableMoss);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateStructureMesh();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FArch_StructureData GetStructureData() const { return StructureData; }

    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    void OnPlayerInteraction();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void RegenerateStructure();

protected:
    UFUNCTION()
    void OnInteractionVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
    void InitializeStructureMeshes();
    void ApplyMaterialBasedOnWeathering();
    void SetupInteractionVolume();
};