#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "Combat_ThreatAssessmentSystem.generated.h"

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "No Threat"),
    Low         UMETA(DisplayName = "Low Threat"),
    Medium      UMETA(DisplayName = "Medium Threat"),
    High        UMETA(DisplayName = "High Threat"),
    Critical    UMETA(DisplayName = "Critical Threat")
};

USTRUCT(BlueprintType)
struct FCombat_ThreatData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    ECombat_ThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    float ThreatRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    float ThreatIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    FVector ThreatOrigin;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    bool bIsActiveThreat;

    FCombat_ThreatData()
    {
        ThreatLevel = ECombat_ThreatLevel::None;
        ThreatRadius = 500.0f;
        ThreatIntensity = 0.0f;
        ThreatOrigin = FVector::ZeroVector;
        bIsActiveThreat = false;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API ACombat_ThreatAssessmentSystem : public AActor
{
    GENERATED_BODY()

public:
    ACombat_ThreatAssessmentSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* DetectionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VisualizationMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    FCombat_ThreatData CurrentThreatData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float AssessmentUpdateRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float MaxDetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    TArray<AActor*> TrackedThreats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    bool bVisualizeThreatZones;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    void UpdateThreatAssessment();

    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    ECombat_ThreatLevel CalculateThreatLevel(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    void AddThreatTarget(AActor* NewThreat);

    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    void RemoveThreatTarget(AActor* ThreatToRemove);

    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    FCombat_ThreatData GetCurrentThreatData() const;

    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    TArray<AActor*> GetThreatsInRange(float Range) const;

    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    bool IsLocationSafe(FVector Location, float SafetyRadius) const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Threat Assessment")
    void OnThreatLevelChanged(ECombat_ThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintImplementableEvent, Category = "Threat Assessment")
    void OnNewThreatDetected(AActor* ThreatActor);

    UFUNCTION(BlueprintImplementableEvent, Category = "Threat Assessment")
    void OnThreatRemoved(AActor* ThreatActor);

protected:
    UFUNCTION()
    void OnDetectionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnDetectionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    FTimerHandle AssessmentTimerHandle;
    ECombat_ThreatLevel PreviousThreatLevel;

    void InitializeComponents();
    void SetupDetectionSphere();
    float CalculateDistanceThreatModifier(float Distance) const;
    float CalculateActorThreatValue(AActor* Actor) const;
    void UpdateVisualization();
};