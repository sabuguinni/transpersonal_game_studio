#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "NPC_PackBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_PackRole : uint8
{
    Leader     UMETA(DisplayName = "Pack Leader"),
    Follower   UMETA(DisplayName = "Pack Follower"),
    Scout      UMETA(DisplayName = "Pack Scout"),
    Guardian   UMETA(DisplayName = "Pack Guardian")
};

USTRUCT(BlueprintType)
struct FNPC_PackMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    TWeakObjectPtr<APawn> MemberPawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    ENPC_PackRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float DistanceFromLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float FormationAngle;

    FNPC_PackMember()
    {
        MemberPawn = nullptr;
        Role = ENPC_PackRole::Follower;
        DistanceFromLeader = 300.0f;
        FormationAngle = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_PackBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_PackBehaviorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Pack Management
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void InitializePack(APawn* LeaderPawn);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void AddPackMember(APawn* NewMember, ENPC_PackRole Role);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void RemovePackMember(APawn* MemberToRemove);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void SetPackFormation(const TArray<FNPC_PackMember>& NewFormation);

    // Pack Behavior
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void UpdatePackFormation();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void ExecutePackHunt(APawn* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void ExecutePackDefense(FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void ExecutePackRetreat(FVector SafeLocation);

    // Pack Communication
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void BroadcastPackSignal(const FString& SignalType);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void ReceivePackSignal(const FString& SignalType, APawn* Sender);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Pack Behavior")
    APawn* GetPackLeader() const { return PackLeader.Get(); }

    UFUNCTION(BlueprintPure, Category = "Pack Behavior")
    TArray<FNPC_PackMember> GetPackMembers() const { return PackMembers; }

    UFUNCTION(BlueprintPure, Category = "Pack Behavior")
    int32 GetPackSize() const { return PackMembers.Num(); }

    UFUNCTION(BlueprintPure, Category = "Pack Behavior")
    bool IsPackLeader() const;

protected:
    // Pack Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    TWeakObjectPtr<APawn> PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    TArray<FNPC_PackMember> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    float MaxPackSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    float FormationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    float CohesionStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    float SeparationStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    float AlignmentStrength;

    // Behavior States
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    bool bIsHunting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    bool bIsDefending;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    bool bIsRetreating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    TWeakObjectPtr<APawn> CurrentTarget;

    // Internal Functions
    void CalculateFlockingForces(APawn* Member, FVector& Cohesion, FVector& Separation, FVector& Alignment);
    void UpdateMemberPosition(FNPC_PackMember& Member);
    void ValidatePackMembers();
};

#include "NPC_PackBehaviorComponent.generated.h"