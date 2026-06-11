#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/TriggerVolume.h"
#include "../SharedTypes.h"
#include "Arch_CretaceousStructuralManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructuralElement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural")
    FString ElementName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural")
    EArch_StructureType StructureType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural")
    FVector Dimensions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural")
    float DurabilityPercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural")
    bool bHasMossGrowth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural")
    bool bIsPartiallyRuined;

    FArch_StructuralElement()
    {
        ElementName = TEXT("Unknown Structure");
        StructureType = EArch_StructureType::Pillar;
        Dimensions = FVector(100, 100, 200);
        DurabilityPercent = 75.0f;
        bHasMossGrowth = true;
        bIsPartiallyRuined = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_InteriorSpace
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FString SpaceName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    EArch_InteriorType InteriorType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FVector SpaceBounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FString> ContainedArtifacts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float AmbientLightLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bHasFirePit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bShowsHabitationSigns;

    FArch_InteriorSpace()
    {
        SpaceName = TEXT("Primitive Shelter");
        InteriorType = EArch_InteriorType::Shelter;
        SpaceBounds = FVector(500, 500, 300);
        AmbientLightLevel = 0.3f;
        bHasFirePit = true;
        bShowsHabitationSigns = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_CretaceousStructuralManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_CretaceousStructuralManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* PrimaryStructureMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteriorTriggerVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Configuration")
    FArch_StructuralElement PrimaryStructure;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Configuration")
    TArray<FArch_StructuralElement> SecondaryElements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Configuration")
    FArch_InteriorSpace InteriorSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    float WeatheringIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    float VegetationOvergrowth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Historical")
    FString ArchaeologicalNotes;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Structural Management")
    void InitializeStructuralElements();

    UFUNCTION(BlueprintCallable, Category = "Structural Management")
    void ApplyWeatheringEffects(float WeatheringAmount);

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    void SetupInteriorSpace(const FArch_InteriorSpace& NewInteriorConfig);

    UFUNCTION(BlueprintCallable, Category = "Environmental")
    void UpdateVegetationGrowth(float GrowthRate);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void OnPlayerEnterInterior();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void OnPlayerExitInterior();

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnStructuralDamageApplied(float DamageAmount);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnInteriorSpaceActivated();

protected:
    UFUNCTION()
    void OnInteriorTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteriorTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

private:
    void UpdateStructuralMaterials();
    void PlaceInteriorArtifacts();
    void CalculateStructuralIntegrity();

    bool bPlayerInInterior;
    float StructuralIntegrity;
    float LastWeatheringUpdate;
};