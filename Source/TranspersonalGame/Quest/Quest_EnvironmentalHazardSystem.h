#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Engine/TimerHandle.h"
#include "SharedTypes.h"
#include "Quest_EnvironmentalHazardSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_HazardType : uint8
{
    None = 0,
    VolcanicEruption,
    FlashFlood,
    Wildfire,
    Earthquake,
    ToxicGas,
    Avalanche,
    Stampede,
    PredatorPack
};

UENUM(BlueprintType)
enum class EQuest_HazardSeverity : uint8
{
    Low = 0,
    Medium,
    High,
    Critical,
    Catastrophic
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_HazardData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EQuest_HazardType HazardType = EQuest_HazardType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EQuest_HazardSeverity Severity = EQuest_HazardSeverity::Low;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AffectedRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector EpicenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DamagePerSecond = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresEvacuation = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> SurvivalTips;

    FQuest_HazardData()
    {
        SurvivalTips.Add("Seek higher ground");
        SurvivalTips.Add("Find shelter immediately");
        SurvivalTips.Add("Avoid open areas");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_HazardResponse
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ResponseName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SuccessChance = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeRequired = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RequiredItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StaminaCost = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresGroupAction = false;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_EnvironmentalHazardSystem : public AActor
{
    GENERATED_BODY()

public:
    AQuest_EnvironmentalHazardSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core hazard management
    UFUNCTION(BlueprintCallable, Category = "Environmental Hazard")
    void TriggerHazard(EQuest_HazardType HazardType, FVector Location, EQuest_HazardSeverity Severity = EQuest_HazardSeverity::Medium);

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazard")
    void EndHazard();

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazard")
    bool IsHazardActive() const { return bHazardActive; }

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazard")
    FQuest_HazardData GetCurrentHazard() const { return CurrentHazard; }

    // Player response system
    UFUNCTION(BlueprintCallable, Category = "Environmental Hazard")
    void AttemptHazardResponse(const FString& ResponseName);

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazard")
    TArray<FQuest_HazardResponse> GetAvailableResponses() const;

    // Environmental monitoring
    UFUNCTION(BlueprintCallable, Category = "Environmental Hazard")
    void StartEnvironmentalMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazard")
    void StopEnvironmentalMonitoring();

    // Quest integration
    UFUNCTION(BlueprintCallable, Category = "Environmental Hazard")
    void GenerateHazardSurvivalQuest();

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazard")
    void GenerateHazardRescueQuest();

protected:
    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* HazardDetectionSphere;

    // Hazard state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard State")
    bool bHazardActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard State")
    FQuest_HazardData CurrentHazard;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard State")
    float HazardTimeRemaining = 0.0f;

    // Response system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response System")
    TArray<FQuest_HazardResponse> AvailableResponses;

    // Monitoring system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring")
    bool bMonitoringActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring")
    float MonitoringInterval = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring")
    float HazardProbability = 0.1f;

    // Timers
    FTimerHandle HazardUpdateTimer;
    FTimerHandle MonitoringTimer;

    // Internal methods
    void UpdateHazard();
    void CheckForEnvironmentalTriggers();
    void ProcessHazardDamage();
    void NotifyPlayersOfHazard();
    void InitializeResponseOptions();
    FQuest_HazardResponse CreateResponse(const FString& Name, float SuccessChance, float TimeRequired, const TArray<FString>& RequiredItems);
};