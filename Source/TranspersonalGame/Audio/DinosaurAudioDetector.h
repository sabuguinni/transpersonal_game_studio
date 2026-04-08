#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AudioManager.h"
#include "DinosaurAudioDetector.generated.h"

USTRUCT(BlueprintType)
struct FDinosaurAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class AActor* DinosaurActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThreatLevel; // 0.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsAggressive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHerbivore;

    FDinosaurAudioData()
    {
        DinosaurActor = nullptr;
        Distance = 0.0f;
        ThreatLevel = 0.0f;
        bIsAggressive = false;
        bIsHerbivore = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurAudioDetector : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurAudioDetector();

protected:
    virtual void BeginPlay() override;

    // Detection Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float UpdateFrequency; // Updates per second

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    TSubclassOf<AActor> DinosaurBaseClass;

    // Audio Response
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class AAudioManager* AudioManager;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float TensionIncreaseRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float TensionDecreaseRate;

    // Current Detection State
    UPROPERTY(BlueprintReadOnly, Category = "Detection State")
    TArray<FDinosaurAudioData> NearbyDinosaurs;

    UPROPERTY(BlueprintReadOnly, Category = "Detection State")
    float CurrentThreatLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Detection State")
    bool bAnyDinosaursDetected;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Detection Functions
    UFUNCTION(BlueprintCallable, Category = "Detection")
    void UpdateDinosaurDetection();

    UFUNCTION(BlueprintCallable, Category = "Detection")
    void SetDetectionRadius(float NewRadius);

    UFUNCTION(BlueprintPure, Category = "Detection")
    float GetClosestDinosaurDistance() const;

    UFUNCTION(BlueprintPure, Category = "Detection")
    float GetHighestThreatLevel() const;

    UFUNCTION(BlueprintPure, Category = "Detection")
    int32 GetDinosaurCount() const { return NearbyDinosaurs.Num(); }

    // Audio Integration
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetAudioManager(AAudioManager* NewAudioManager);

private:
    float LastUpdateTime;
    float UpdateInterval;

    void CalculateThreatLevel();
    void UpdateAudioSystem();
    float CalculateIndividualThreat(const FDinosaurAudioData& DinosaurData) const;
};