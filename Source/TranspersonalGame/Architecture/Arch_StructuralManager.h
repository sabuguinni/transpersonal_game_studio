#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Arch_StructuralManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    StonePillar     UMETA(DisplayName = "Stone Pillar"),
    StoneWall       UMETA(DisplayName = "Stone Wall"),
    StoneArchway    UMETA(DisplayName = "Stone Archway"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    RockFormation   UMETA(DisplayName = "Rock Formation"),
    AncientRuin     UMETA(DisplayName = "Ancient Ruin")
};

USTRUCT(BlueprintType)
struct FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::StonePillar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Dimensions = FVector(200.0f, 200.0f, 400.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatheringLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasMossGrowth = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasCarvings = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructuralIntegrity = 1.0f;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::StonePillar;
        Dimensions = FVector(200.0f, 200.0f, 400.0f);
        WeatheringLevel = 0.5f;
        bHasMossGrowth = true;
        bHasCarvings = false;
        StructuralIntegrity = 1.0f;
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
    UStaticMeshComponent* StructureMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteractionVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Config")
    FArch_StructureData StructureConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* StoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* WeatheredStoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* MossyStoneMaterial;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void InitializeStructure(EArch_StructureType Type, FVector Location, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void ApplyWeathering(float WeatheringAmount);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void SetMossGrowth(bool bEnableMoss);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void AddCarvings(bool bEnableCarvings);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    float GetStructuralIntegrity() const { return StructureConfig.StructuralIntegrity; }

    UFUNCTION(BlueprintCallable, Category = "Structure")
    EArch_StructureType GetStructureType() const { return StructureConfig.StructureType; }

    UFUNCTION(BlueprintImplementableEvent, Category = "Structure")
    void OnStructureInteraction(AActor* InteractingActor);

    UFUNCTION(BlueprintImplementableEvent, Category = "Structure")
    void OnStructuralDamage(float DamageAmount);

protected:
    UFUNCTION()
    void OnInteractionVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
    void UpdateStructureMaterial();
    void SetStructureDimensions();
    UStaticMesh* GetMeshForStructureType(EArch_StructureType Type);
};