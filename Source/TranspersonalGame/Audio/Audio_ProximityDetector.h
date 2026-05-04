#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "Audio_ProximityDetector.generated.h"

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "Sem Ameaça"),
    Low         UMETA(DisplayName = "Ameaça Baixa"),
    Medium      UMETA(DisplayName = "Ameaça Média"),
    High        UMETA(DisplayName = "Ameaça Alta"),
    Critical    UMETA(DisplayName = "Ameaça Crítica")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ProximityData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
    float Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
    EAudio_ThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
    FString TargetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
    float AudioIntensity;

    FAudio_ProximityData()
    {
        Distance = 0.0f;
        ThreatLevel = EAudio_ThreatLevel::None;
        TargetName = TEXT("Unknown");
        TargetLocation = FVector::ZeroVector;
        AudioIntensity = 0.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProximityAlert, const FAudio_ProximityData&, ProximityData);

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ProximityDetector : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ProximityDetector();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Proximity Detector")
    void SetDetectionRadius(float NewRadius);

    UFUNCTION(BlueprintCallable, Category = "Proximity Detector")
    void AddTargetTag(const FString& Tag);

    UFUNCTION(BlueprintCallable, Category = "Proximity Detector")
    void RemoveTargetTag(const FString& Tag);

    UFUNCTION(BlueprintPure, Category = "Proximity Detector")
    TArray<FAudio_ProximityData> GetNearbyThreats() const { return NearbyThreats; }

    UPROPERTY(BlueprintAssignable, Category = "Proximity Events")
    FOnProximityAlert OnProximityAlert;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection Settings")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection Settings")
    float UpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection Settings")
    TArray<FString> TargetTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Levels")
    float CriticalDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Levels")
    float HighDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Levels")
    float MediumDistance;

    UPROPERTY(BlueprintReadOnly, Category = "Detection State")
    TArray<FAudio_ProximityData> NearbyThreats;

    UPROPERTY(BlueprintReadOnly, Category = "Detection State")
    float LastUpdateTime;

private:
    void UpdateProximityDetection();
    EAudio_ThreatLevel CalculateThreatLevel(float Distance) const;
    float CalculateAudioIntensity(float Distance, EAudio_ThreatLevel ThreatLevel) const;
    void ProcessDetectedActor(AActor* Actor);
};