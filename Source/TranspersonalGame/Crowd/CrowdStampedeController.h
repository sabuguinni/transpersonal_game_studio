// CrowdStampedeController.h
// Crowd & Traffic Simulation Agent #13
// Controls stampede events — mass panic, directional flee, scatter patterns

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdStampedeController.generated.h"

UENUM(BlueprintType)
enum class ECrowd_StampedeType : uint8
{
    PanicFlee        UMETA(DisplayName = "Panic Flee"),
    DirectionalRush  UMETA(DisplayName = "Directional Rush"),
    Scatter          UMETA(DisplayName = "Scatter"),
    CircleStampede   UMETA(DisplayName = "Circle Stampede")
};

USTRUCT(BlueprintType)
struct FCrowd_StampedeEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stampede")
    FVector TriggerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stampede")
    ECrowd_StampedeType StampedeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stampede")
    float PanicRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stampede")
    float StampedeSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stampede")
    float DurationSeconds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stampede")
    int32 AffectedAgentCount;

    FCrowd_StampedeEvent()
        : TriggerLocation(FVector::ZeroVector)
        , StampedeType(ECrowd_StampedeType::PanicFlee)
        , PanicRadius(500.f)
        , StampedeSpeed(800.f)
        , DurationSeconds(15.f)
        , AffectedAgentCount(0)
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdStampedeController : public AActor
{
    GENERATED_BODY()

public:
    ACrowdStampedeController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void TriggerStampede(FVector Origin, ECrowd_StampedeType Type, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void StopStampede();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    bool IsStampedeActive() const { return bStampedeActive; }

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    FVector ComputeFleeDirection(FVector AgentLocation, FVector PanicOrigin) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    int32 GetAffectedAgentCount() const { return ActiveEvent.AffectedAgentCount; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float DefaultPanicRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float DefaultStampedeSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float MaxStampedeDuration;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Stampede")
    FCrowd_StampedeEvent ActiveEvent;

private:
    bool bStampedeActive;
    float StampedeElapsed;

    void UpdateStampedeAgents(float DeltaTime);
    FVector ComputeScatterDirection(FVector AgentLocation, FVector Origin) const;
    FVector ComputeCircleDirection(FVector AgentLocation, FVector Origin) const;
};
