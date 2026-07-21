#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Arch_VolcanicStructureSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_VolcanicStructureType : uint8
{
    LavaRock,
    VolcanicPillar,
    LavaFlow,
    VolcanicCave,
    ObsidianFormation
};

USTRUCT(BlueprintType)
struct FArch_VolcanicStructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Structure")
    EArch_VolcanicStructureType StructureType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Structure")
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Structure")
    bool bIsActiveFlow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Structure")
    float DamageRadius;

    FArch_VolcanicStructureData()
    {
        StructureType = EArch_VolcanicStructureType::LavaRock;
        Temperature = 800.0f;
        bIsActiveFlow = false;
        DamageRadius = 500.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_VolcanicStructureSystem : public AActor
{
    GENERATED_BODY()

public:
    AArch_VolcanicStructureSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StructureMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* HeatZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Structure")
    FArch_VolcanicStructureData StructureData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Structure")
    TArray<UStaticMesh*> VolcanicMeshes;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Volcanic Structure")
    void SetStructureType(EArch_VolcanicStructureType NewType);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Structure")
    void ActivateLavaFlow();

    UFUNCTION(BlueprintCallable, Category = "Volcanic Structure")
    void DeactivateLavaFlow();

    UFUNCTION(BlueprintPure, Category = "Volcanic Structure")
    float GetTemperature() const;

    UFUNCTION(BlueprintCallable, Category = "Volcanic Structure")
    void SpawnVolcanicDebris();

protected:
    UFUNCTION()
    void OnHeatZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnHeatZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    void UpdateStructureAppearance();
    void UpdateHeatEffects();
    
    float CurrentHeatIntensity;
    FTimerHandle LavaFlowTimer;
};