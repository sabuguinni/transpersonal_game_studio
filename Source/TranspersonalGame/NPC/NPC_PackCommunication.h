#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "NPC_PackCommunication.generated.h"

UENUM(BlueprintType)
enum class ENPC_PackSignal : uint8
{
    None            UMETA(DisplayName = "None"),
    TargetSpotted   UMETA(DisplayName = "Target Spotted"),
    Danger          UMETA(DisplayName = "Danger"),
    FoodFound       UMETA(DisplayName = "Food Found"),
    Regroup         UMETA(DisplayName = "Regroup"),
    Attack          UMETA(DisplayName = "Attack"),
    Retreat         UMETA(DisplayName = "Retreat"),
    Patrol          UMETA(DisplayName = "Patrol"),
    Rest            UMETA(DisplayName = "Rest")
};

USTRUCT(BlueprintType)
struct FNPC_PackMessage
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Communication")
    ENPC_PackSignal Signal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Communication")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Communication")
    AActor* TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Communication")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Communication")
    float Timestamp;

    FNPC_PackMessage()
    {
        Signal = ENPC_PackSignal::None;
        TargetLocation = FVector::ZeroVector;
        TargetActor = nullptr;
        Priority = 0.0f;
        Timestamp = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_PackCommunication : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_PackCommunication();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Pack Communication
    UFUNCTION(BlueprintCallable, Category = "Pack Communication")
    void SendPackSignal(ENPC_PackSignal Signal, FVector TargetLocation = FVector::ZeroVector, AActor* TargetActor = nullptr, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Pack Communication")
    void ReceivePackMessage(const FNPC_PackMessage& Message);

    UFUNCTION(BlueprintCallable, Category = "Pack Communication")
    void JoinPack(AActor* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack Communication")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack Communication")
    bool IsInPack() const { return PackLeader != nullptr; }

    UFUNCTION(BlueprintCallable, Category = "Pack Communication")
    AActor* GetPackLeader() const { return PackLeader; }

    UFUNCTION(BlueprintCallable, Category = "Pack Communication")
    TArray<AActor*> GetPackMembers() const { return PackMembers; }

    // Pack Behavior
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void UpdatePackCohesion();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    FVector GetPackCenterLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    float GetDistanceToPackCenter() const;

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    bool ShouldMaintainFormation() const;

protected:
    // Pack Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    float CommunicationRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    float CohesionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    float FormationDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    int32 MaxPackSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    bool bIsPackLeader;

    // Pack State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack State")
    AActor* PackLeader;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack State")
    TArray<AActor*> PackMembers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack State")
    TArray<FNPC_PackMessage> RecentMessages;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack State")
    float LastCommunicationTime;

    // Internal Functions
    void BroadcastToPackMembers(const FNPC_PackMessage& Message);
    void UpdatePackMembersList();
    void ProcessMessages();
    bool IsActorInRange(AActor* Actor, float Range) const;
    void CleanupOldMessages();

private:
    float MessageRetentionTime;
    float LastPackUpdate;
    float PackUpdateInterval;
};