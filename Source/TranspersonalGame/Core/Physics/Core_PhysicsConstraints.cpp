#include "Core_PhysicsConstraints.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"

UCore_PhysicsConstraints::UCore_PhysicsConstraints()
{
    PrimaryComponentTick.bCanEverTick = true;
    bAutoCreateConstraints = false;
    DefaultBreakForce = 10000.0f;
}

void UCore_PhysicsConstraints::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoCreateConstraints && ConstraintSettings.Num() > 0)
    {
        // Auto-create constraints based on settings
        for (const FCore_ConstraintSettings& Settings : ConstraintSettings)
        {
            // This would need specific actors to connect - placeholder for now
            UE_LOG(LogTemp, Log, TEXT("Auto-creating constraint of type: %d"), (int32)Settings.ConstraintType);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsConstraints initialized with %d constraint settings"), ConstraintSettings.Num());
}

void UCore_PhysicsConstraints::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Check for broken constraints
    CheckForBrokenConstraints();
}

UPhysicsConstraintComponent* UCore_PhysicsConstraints::CreateConstraint(const FCore_ConstraintSettings& Settings, 
                                                                        AActor* Actor1, AActor* Actor2)
{
    if (!Actor1 || !Actor2)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot create constraint: one or both actors are null"));
        return nullptr;
    }

    UPhysicsConstraintComponent* NewConstraint = CreateConstraintComponent();
    if (!NewConstraint)
    {
        return nullptr;
    }

    // Set constraint actors
    NewConstraint->SetConstrainedComponents(
        Actor1->FindComponentByClass<UPrimitiveComponent>(),
        NAME_None,
        Actor2->FindComponentByClass<UPrimitiveComponent>(),
        NAME_None
    );

    // Apply settings
    SetupConstraintFromSettings(NewConstraint, Settings);

    // Add to active constraints
    ActiveConstraints.Add(NewConstraint);

    UE_LOG(LogTemp, Log, TEXT("Created constraint between %s and %s"), *Actor1->GetName(), *Actor2->GetName());

    return NewConstraint;
}

void UCore_PhysicsConstraints::RemoveConstraint(UPhysicsConstraintComponent* Constraint)
{
    if (!Constraint)
    {
        return;
    }

    ActiveConstraints.Remove(Constraint);
    Constraint->DestroyComponent();

    UE_LOG(LogTemp, Log, TEXT("Removed physics constraint"));
}

void UCore_PhysicsConstraints::RemoveAllConstraints()
{
    for (UPhysicsConstraintComponent* Constraint : ActiveConstraints)
    {
        if (Constraint)
        {
            Constraint->DestroyComponent();
        }
    }
    
    ActiveConstraints.Empty();
    UE_LOG(LogTemp, Log, TEXT("Removed all physics constraints"));
}

void UCore_PhysicsConstraints::SetConstraintBreakForce(UPhysicsConstraintComponent* Constraint, float NewBreakForce)
{
    if (!Constraint)
    {
        return;
    }

    Constraint->SetLinearBreakable(true, NewBreakForce);
    UE_LOG(LogTemp, Log, TEXT("Set constraint break force to: %f"), NewBreakForce);
}

void UCore_PhysicsConstraints::SetConstraintBreakTorque(UPhysicsConstraintComponent* Constraint, float NewBreakTorque)
{
    if (!Constraint)
    {
        return;
    }

    Constraint->SetAngularBreakable(true, NewBreakTorque);
    UE_LOG(LogTemp, Log, TEXT("Set constraint break torque to: %f"), NewBreakTorque);
}

TArray<UPhysicsConstraintComponent*> UCore_PhysicsConstraints::GetActiveConstraints() const
{
    return ActiveConstraints;
}

int32 UCore_PhysicsConstraints::GetConstraintCount() const
{
    return ActiveConstraints.Num();
}

bool UCore_PhysicsConstraints::IsConstraintBroken(UPhysicsConstraintComponent* Constraint) const
{
    if (!Constraint)
    {
        return true;
    }

    return Constraint->IsBroken();
}

void UCore_PhysicsConstraints::CreateRagdollConstraints(AActor* TargetActor)
{
    if (!TargetActor)
    {
        return;
    }

    USkeletalMeshComponent* SkeletalMesh = TargetActor->FindComponentByClass<USkeletalMeshComponent>();
    if (!SkeletalMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot create ragdoll constraints: no skeletal mesh found"));
        return;
    }

    // Enable ragdoll physics
    SkeletalMesh->SetSimulatePhysics(true);
    SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    UE_LOG(LogTemp, Log, TEXT("Created ragdoll constraints for actor: %s"), *TargetActor->GetName());
}

void UCore_PhysicsConstraints::CreateChainConstraints(const TArray<AActor*>& ChainActors)
{
    if (ChainActors.Num() < 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("Need at least 2 actors to create chain constraints"));
        return;
    }

    FCore_ConstraintSettings ChainSettings;
    ChainSettings.ConstraintType = ECore_ConstraintType::Ball;
    ChainSettings.bEnableLinearLimits = true;
    ChainSettings.LinearLimitSize = FVector(50.0f, 50.0f, 50.0f);
    ChainSettings.BreakForce = DefaultBreakForce;

    for (int32 i = 0; i < ChainActors.Num() - 1; i++)
    {
        CreateConstraint(ChainSettings, ChainActors[i], ChainActors[i + 1]);
    }

    UE_LOG(LogTemp, Log, TEXT("Created chain constraints for %d actors"), ChainActors.Num());
}

void UCore_PhysicsConstraints::SetupConstraintFromSettings(UPhysicsConstraintComponent* Constraint, const FCore_ConstraintSettings& Settings)
{
    if (!Constraint)
    {
        return;
    }

    // Set constraint type behavior
    switch (Settings.ConstraintType)
    {
    case ECore_ConstraintType::Fixed:
        Constraint->SetLinearXMotion(ELinearConstraintMotion::LCM_Locked);
        Constraint->SetLinearYMotion(ELinearConstraintMotion::LCM_Locked);
        Constraint->SetLinearZMotion(ELinearConstraintMotion::LCM_Locked);
        Constraint->SetAngularSwing1Motion(EAngularConstraintMotion::ACM_Locked);
        Constraint->SetAngularSwing2Motion(EAngularConstraintMotion::ACM_Locked);
        Constraint->SetAngularTwistMotion(EAngularConstraintMotion::ACM_Locked);
        break;
        
    case ECore_ConstraintType::Hinge:
        Constraint->SetLinearXMotion(ELinearConstraintMotion::LCM_Locked);
        Constraint->SetLinearYMotion(ELinearConstraintMotion::LCM_Locked);
        Constraint->SetLinearZMotion(ELinearConstraintMotion::LCM_Locked);
        Constraint->SetAngularSwing1Motion(EAngularConstraintMotion::ACM_Locked);
        Constraint->SetAngularSwing2Motion(EAngularConstraintMotion::ACM_Locked);
        Constraint->SetAngularTwistMotion(EAngularConstraintMotion::ACM_Free);
        break;
        
    case ECore_ConstraintType::Ball:
        Constraint->SetLinearXMotion(ELinearConstraintMotion::LCM_Locked);
        Constraint->SetLinearYMotion(ELinearConstraintMotion::LCM_Locked);
        Constraint->SetLinearZMotion(ELinearConstraintMotion::LCM_Locked);
        Constraint->SetAngularSwing1Motion(EAngularConstraintMotion::ACM_Free);
        Constraint->SetAngularSwing2Motion(EAngularConstraintMotion::ACM_Free);
        Constraint->SetAngularTwistMotion(EAngularConstraintMotion::ACM_Free);
        break;
        
    default:
        break;
    }

    // Set linear limits
    if (Settings.bEnableLinearLimits)
    {
        Constraint->SetLinearXMotion(ELinearConstraintMotion::LCM_Limited);
        Constraint->SetLinearYMotion(ELinearConstraintMotion::LCM_Limited);
        Constraint->SetLinearZMotion(ELinearConstraintMotion::LCM_Limited);
        Constraint->SetLinearXLimit(ELinearConstraintMotion::LCM_Limited, Settings.LinearLimitSize.X);
        Constraint->SetLinearYLimit(ELinearConstraintMotion::LCM_Limited, Settings.LinearLimitSize.Y);
        Constraint->SetLinearZLimit(ELinearConstraintMotion::LCM_Limited, Settings.LinearLimitSize.Z);
    }

    // Set angular limits
    if (Settings.bEnableAngularLimits)
    {
        Constraint->SetAngularSwing1Motion(EAngularConstraintMotion::ACM_Limited);
        Constraint->SetAngularSwing2Motion(EAngularConstraintMotion::ACM_Limited);
        Constraint->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Limited, Settings.AngularLimitAngle);
        Constraint->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Limited, Settings.AngularLimitAngle);
    }

    // Set break forces
    Constraint->SetLinearBreakable(true, Settings.BreakForce);
    Constraint->SetAngularBreakable(true, Settings.BreakTorque);

    // Set collision
    Constraint->SetDisableCollision(!Settings.bEnableCollision);
}

void UCore_PhysicsConstraints::CheckForBrokenConstraints()
{
    for (int32 i = ActiveConstraints.Num() - 1; i >= 0; i--)
    {
        UPhysicsConstraintComponent* Constraint = ActiveConstraints[i];
        if (!Constraint || Constraint->IsBroken())
        {
            if (Constraint)
            {
                OnConstraintBroken(Constraint);
                UE_LOG(LogTemp, Warning, TEXT("Constraint broken and removed"));
            }
            ActiveConstraints.RemoveAt(i);
        }
    }
}

UPhysicsConstraintComponent* UCore_PhysicsConstraints::CreateConstraintComponent()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return nullptr;
    }

    UPhysicsConstraintComponent* NewConstraint = NewObject<UPhysicsConstraintComponent>(Owner);
    if (NewConstraint)
    {
        NewConstraint->AttachToComponent(Owner->GetRootComponent(), 
                                       FAttachmentTransformRules::KeepWorldTransform);
        NewConstraint->RegisterComponent();
    }

    return NewConstraint;
}