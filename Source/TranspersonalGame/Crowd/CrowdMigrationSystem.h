#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdMigrationSystem.generated.h"

UENUM(BlueprintType)
enum class ECrowd_MigrationState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Migrating   UMETA(DisplayName = "Migrating"),
    Dispersing  UMETA(DisplayName = "Dispersing"),
    Fleeing     UMETA(DisplayName = "Fleeing")
};

USTRUCT(BlueprintType)
struct FCrowd_MigrationAgent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    FVector CurrentLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    float MoveSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    ECrowd_MigrationState State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    int32 HerdID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    bool bIsLeader;

    FCrowd_MigrationAgent()
        : CurrentLocation(FVector::ZeroVector)
        , TargetLocation(FVector::ZeroVector)
        , MoveSpeed(300.f)
        , State(ECrowd_MigrationState::Idle)
        , HerdID(0)
        , bIsLeader(false)
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdMigrationSystem : public AActor
{
    GENERATED_BODY()

public:
    ACrowdMigrationSystem();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    int32 HerdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    float MigrationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    float FleeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    FVector MigrationDestination;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    TArray<FCrowd_MigrationAgent> Agents;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Migration")
    void InitializeHerd(int32 InHerdSize, FVector Origin, FVector Destination);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Migration")
    void TriggerFlee(FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Migration")
    void UpdateAgentPositions(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Migration")
    int32 GetActiveAgentCount() const;

private:
    float ElapsedTime;
    bool bMigrationActive;
};
