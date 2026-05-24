#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerBox.h"
#include "GameFramework/Pawn.h"
#include "Combat_AlertSystem.generated.h"

UENUM(BlueprintType)
enum class ECombat_AlertLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Caution     UMETA(DisplayName = "Caution"), 
    Danger      UMETA(DisplayName = "Danger"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FCombat_ThreatData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    AActor* ThreatActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    float Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    ECombat_AlertLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    FVector LastKnownLocation;

    FCombat_ThreatData()
    {
        ThreatActor = nullptr;
        Distance = 0.0f;
        ThreatLevel = ECombat_AlertLevel::Safe;
        LastKnownLocation = FVector::ZeroVector;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_AlertSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_AlertSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Alert System Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alert System")
    ECombat_AlertLevel CurrentAlertLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alert System")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alert System")
    float CriticalThreatDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alert System")
    float AlertDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alert System")
    TArray<FCombat_ThreatData> ActiveThreats;

    // Detection Methods
    UFUNCTION(BlueprintCallable, Category = "Alert System")
    void ScanForThreats();

    UFUNCTION(BlueprintCallable, Category = "Alert System")
    void AddThreat(AActor* ThreatActor, ECombat_AlertLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Alert System")
    void RemoveThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Alert System")
    void UpdateAlertLevel();

    // Alert Response Methods
    UFUNCTION(BlueprintCallable, Category = "Alert System")
    void TriggerAlert(ECombat_AlertLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Alert System")
    FVector GetNearestEscapePoint();

    UFUNCTION(BlueprintCallable, Category = "Alert System")
    bool IsInDanger() const;

    // Event Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAlertLevelChanged, ECombat_AlertLevel, NewLevel);
    UPROPERTY(BlueprintAssignable, Category = "Alert System")
    FOnAlertLevelChanged OnAlertLevelChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnThreatDetected, AActor*, ThreatActor);
    UPROPERTY(BlueprintAssignable, Category = "Alert System")
    FOnThreatDetected OnThreatDetected;

private:
    // Internal tracking
    float LastScanTime;
    float ScanInterval;
    
    // Escape points cache
    TArray<FVector> EscapePoints;
    
    void InitializeEscapePoints();
    float CalculateThreatLevel(AActor* ThreatActor, float Distance);
    bool IsDinosaurThreat(AActor* Actor);
};