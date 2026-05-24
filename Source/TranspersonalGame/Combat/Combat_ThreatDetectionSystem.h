#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerSphere.h"
#include "SharedTypes.h"
#include "Combat_ThreatDetectionSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_ThreatLevel
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    float ThreatRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    float DamagePerSecond = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    EThreatLevel Level = EThreatLevel::Low;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    FString ThreatName = TEXT("Unknown");

    FCombat_ThreatLevel()
    {
        ThreatRadius = 1000.0f;
        DamagePerSecond = 10.0f;
        Level = EThreatLevel::Low;
        ThreatName = TEXT("Unknown");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_DetectionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    AActor* ThreatActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float Distance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    FVector Direction = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float DetectionTime = 0.0f;

    FCombat_DetectionData()
    {
        ThreatActor = nullptr;
        Distance = 0.0f;
        Direction = FVector::ZeroVector;
        DetectionTime = 0.0f;
    }
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_ThreatDetectionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_ThreatDetectionSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Threat detection properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (AllowPrivateAccess = "true"))
    float ScanRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (AllowPrivateAccess = "true"))
    float ScanInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (AllowPrivateAccess = "true"))
    TArray<FCombat_ThreatLevel> ThreatDatabase;

    UPROPERTY(BlueprintReadOnly, Category = "Detection", meta = (AllowPrivateAccess = "true"))
    TArray<FCombat_DetectionData> ActiveThreats;

    UPROPERTY(BlueprintReadOnly, Category = "Detection", meta = (AllowPrivateAccess = "true"))
    EThreatLevel CurrentThreatLevel = EThreatLevel::None;

private:
    float LastScanTime = 0.0f;
    class ATranspersonalCharacter* PlayerCharacter = nullptr;

public:
    // Threat detection methods
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ScanForThreats();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FCombat_ThreatLevel GetThreatLevel(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsActorThreat(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector GetNearestThreatDirection() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetNearestThreatDistance() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AddThreatType(const FCombat_ThreatLevel& NewThreat);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ClearThreats();

    // Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void OnThreatDetected(const FCombat_DetectionData& ThreatData);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void OnThreatLevelChanged(EThreatLevel NewLevel);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void OnThreatLost(AActor* ThreatActor);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Combat")
    FORCEINLINE TArray<FCombat_DetectionData> GetActiveThreats() const { return ActiveThreats; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    FORCEINLINE EThreatLevel GetCurrentThreatLevel() const { return CurrentThreatLevel; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    FORCEINLINE float GetScanRadius() const { return ScanRadius; }
};

#include "Combat_ThreatDetectionSystem.generated.h"