#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "NPC_DinosaurCommunicationSystem.generated.h"

// Communication signal types
UENUM(BlueprintType)
enum class ENPC_CommunicationType : uint8
{
    None = 0,
    Warning,          // Danger alert
    Mating,           // Mating call
    Territorial,      // Territory claim
    Feeding,          // Food source found
    Distress,         // Help needed
    Social,           // General social interaction
    Hunting,          // Coordinated hunting
    Herding,          // Group movement coordination
    Parental          // Parent-offspring communication
};

// Communication method types
UENUM(BlueprintType)
enum class ENPC_CommunicationMethod : uint8
{
    Vocalization = 0, // Roars, calls, growls
    Visual,           // Body language, displays
    Chemical,         // Scent marking
    Physical,         // Touch, nudging
    Behavioral        // Actions that communicate intent
};

// Communication signal structure
USTRUCT(BlueprintType)
struct FNPC_CommunicationSignal
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    ENPC_CommunicationType SignalType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    ENPC_CommunicationMethod Method;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    float Range;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    FVector SourceLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    AActor* Sender;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    AActor* IntendedReceiver;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    FString Message;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    float Timestamp;

    FNPC_CommunicationSignal()
    {
        SignalType = ENPC_CommunicationType::None;
        Method = ENPC_CommunicationMethod::Vocalization;
        Intensity = 1.0f;
        Range = 2000.0f;
        SourceLocation = FVector::ZeroVector;
        Sender = nullptr;
        IntendedReceiver = nullptr;
        Message = TEXT("");
        Timestamp = 0.0f;
    }
};

// Communication response structure
USTRUCT(BlueprintType)
struct FNPC_CommunicationResponse
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    bool bShouldRespond;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    ENPC_CommunicationType ResponseType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    float ResponseDelay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    float ResponseIntensity;

    FNPC_CommunicationResponse()
    {
        bShouldRespond = false;
        ResponseType = ENPC_CommunicationType::None;
        ResponseDelay = 0.0f;
        ResponseIntensity = 1.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCommunicationSignalReceived, const FNPC_CommunicationSignal&, Signal, AActor*, Receiver);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCommunicationResponse, AActor*, Responder, const FNPC_CommunicationSignal&, OriginalSignal, const FNPC_CommunicationResponse&, Response);

/**
 * Advanced dinosaur communication system that handles complex inter-species and intra-species communication
 * Manages vocalizations, visual displays, chemical signals, and behavioral communication
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DinosaurCommunicationSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurCommunicationSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Communication transmission
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Communication")
    void SendCommunicationSignal(ENPC_CommunicationType SignalType, ENPC_CommunicationMethod Method, float Intensity = 1.0f, AActor* IntendedReceiver = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Communication")
    void BroadcastSignal(const FNPC_CommunicationSignal& Signal);

    // Communication reception
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Communication")
    void ReceiveCommunicationSignal(const FNPC_CommunicationSignal& Signal);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Communication")
    FNPC_CommunicationResponse ProcessSignal(const FNPC_CommunicationSignal& Signal);

    // Species-specific communication
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Communication")
    void InitializeSpeciesCommunication(EDinosaurSpecies Species);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Communication")
    bool CanUnderstandSpecies(EDinosaurSpecies OtherSpecies) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Communication")
    float GetCommunicationCompatibility(AActor* OtherDinosaur) const;

    // Vocalization system
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Communication")
    void PlayVocalization(ENPC_CommunicationType SignalType, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Communication")
    void StopVocalization();

    // Visual communication
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Communication")
    void PerformVisualDisplay(ENPC_CommunicationType SignalType, AActor* Target = nullptr);

    // Chemical communication
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Communication")
    void LeaveScentMark(ENPC_CommunicationType SignalType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Communication")
    void DetectScent(const FVector& Location, float DetectionRadius);

    // Communication queries
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Communication")
    TArray<FNPC_CommunicationSignal> GetActiveSignalsInRange(float Range) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Communication")
    bool IsReceivingSignalType(ENPC_CommunicationType SignalType) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Communication")
    AActor* FindNearestCommunicatingDinosaur(ENPC_CommunicationType SignalType, float MaxRange) const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Dinosaur Communication")
    FOnCommunicationSignalReceived OnSignalReceived;

    UPROPERTY(BlueprintAssignable, Category = "Dinosaur Communication")
    FOnCommunicationResponse OnCommunicationResponse;

protected:
    // Communication parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication Settings")
    EDinosaurSpecies OwnerSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication Settings")
    float MaxCommunicationRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication Settings")
    float VocalizationVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication Settings")
    float CommunicationFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication Settings")
    TMap<ENPC_CommunicationType, float> SignalRanges;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication Settings")
    TMap<EDinosaurSpecies, float> SpeciesCompatibility;

    // Active signals
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Communication State")
    TArray<FNPC_CommunicationSignal> ActiveSignals;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Communication State")
    TArray<FNPC_CommunicationSignal> ReceivedSignals;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Communication State")
    bool bIsVocalizing;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Communication State")
    ENPC_CommunicationType CurrentVocalizationType;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Communication State")
    float LastCommunicationTime;

private:
    // Internal systems
    UWorld* World;
    
    // Communication processing
    void UpdateActiveCommunications(float DeltaTime);
    void ProcessIncomingSignals(float DeltaTime);
    void CleanupExpiredSignals();
    
    // Signal generation
    FNPC_CommunicationSignal CreateSignal(ENPC_CommunicationType SignalType, ENPC_CommunicationMethod Method, float Intensity, AActor* IntendedReceiver);
    float CalculateSignalRange(ENPC_CommunicationType SignalType, float Intensity) const;
    
    // Response generation
    FNPC_CommunicationResponse GenerateResponse(const FNPC_CommunicationSignal& Signal);
    bool ShouldRespondToSignal(const FNPC_CommunicationSignal& Signal) const;
    
    // Species-specific behavior
    void LoadSpeciesCommunicationData(EDinosaurSpecies Species);
    float GetSignalIntensityModifier(ENPC_CommunicationType SignalType) const;
    
    // Communication helpers
    TArray<AActor*> FindNearbyDinosaurs(float Range) const;
    bool IsInCommunicationRange(AActor* Target, float SignalRange) const;
    float CalculateSignalAttenuation(float Distance, float MaxRange) const;
};