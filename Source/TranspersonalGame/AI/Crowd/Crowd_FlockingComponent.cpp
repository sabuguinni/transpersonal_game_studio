#include "Crowd_FlockingComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/KismetMathLibrary.h"

UCrowd_FlockingComponent::UCrowd_FlockingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    CurrentVelocity = FVector::ZeroVector;
    TargetVelocity = FVector::ZeroVector;
}

void UCrowd_FlockingComponent::BeginPlay()
{
    Super::BeginPlay();
    
    if (APawn* Owner = Cast<APawn>(GetOwner()))
    {
        CurrentVelocity = Owner->GetVelocity();
    }
}

void UCrowd_FlockingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bEnableFlocking || FlockMembers.Num() == 0)
    {
        return;
    }

    APawn* Owner = Cast<APawn>(GetOwner());
    if (!Owner)
    {
        return;
    }

    // Calculate flocking forces
    FVector FlockingForce = CalculateFlockingForce();
    
    // Apply forces to velocity
    TargetVelocity += FlockingForce * DeltaTime;
    TargetVelocity = TargetVelocity.GetClampedToMaxSize(FlockingData.MaxSpeed);
    
    // Smooth velocity transition
    CurrentVelocity = FMath::VInterpTo(CurrentVelocity, TargetVelocity, DeltaTime, 2.0f);
    
    // Apply movement to pawn
    if (CurrentVelocity.SizeSquared() > 1.0f)
    {
        FVector NewLocation = Owner->GetActorLocation() + (CurrentVelocity * DeltaTime);
        Owner->SetActorLocation(NewLocation, true);
        
        // Orient towards movement direction
        FRotator NewRotation = UKismetMathLibrary::FindLookAtRotation(Owner->GetActorLocation(), NewLocation);
        Owner->SetActorRotation(FMath::RInterpTo(Owner->GetActorRotation(), NewRotation, DeltaTime, 3.0f));
    }
}

void UCrowd_FlockingComponent::AddFlockMember(APawn* NewMember)
{
    if (NewMember && !FlockMembers.Contains(NewMember))
    {
        FlockMembers.Add(NewMember);
    }
}

void UCrowd_FlockingComponent::RemoveFlockMember(APawn* Member)
{
    FlockMembers.Remove(Member);
}

FVector UCrowd_FlockingComponent::CalculateFlockingForce()
{
    FVector Separation = CalculateSeparation() * FlockingData.SeparationWeight;
    FVector Alignment = CalculateAlignment() * FlockingData.AlignmentWeight;
    FVector Cohesion = CalculateCohesion() * FlockingData.CohesionWeight;
    
    FVector TotalForce = Separation + Alignment + Cohesion;
    return TotalForce.GetClampedToMaxSize(FlockingData.MaxForce);
}

FVector UCrowd_FlockingComponent::CalculateSeparation()
{
    APawn* Owner = Cast<APawn>(GetOwner());
    if (!Owner)
    {
        return FVector::ZeroVector;
    }

    FVector OwnerLocation = Owner->GetActorLocation();
    FVector SeparationForce = FVector::ZeroVector;
    int32 Count = 0;

    TArray<APawn*> NearbyMembers = GetNearbyFlockMembers(FlockingData.SeparationRadius);
    
    for (APawn* Member : NearbyMembers)
    {
        if (Member && Member != Owner)
        {
            FVector ToMember = OwnerLocation - Member->GetActorLocation();
            float Distance = ToMember.Size();
            
            if (Distance > 0.0f && Distance < FlockingData.SeparationRadius)
            {
                ToMember.Normalize();
                ToMember /= Distance; // Weight by distance
                SeparationForce += ToMember;
                Count++;
            }
        }
    }

    if (Count > 0)
    {
        SeparationForce /= Count;
        SeparationForce.Normalize();
        SeparationForce *= FlockingData.MaxSpeed;
        SeparationForce -= CurrentVelocity;
    }

    return SeparationForce;
}

FVector UCrowd_FlockingComponent::CalculateAlignment()
{
    APawn* Owner = Cast<APawn>(GetOwner());
    if (!Owner)
    {
        return FVector::ZeroVector;
    }

    FVector AverageVelocity = FVector::ZeroVector;
    int32 Count = 0;

    TArray<APawn*> NearbyMembers = GetNearbyFlockMembers(FlockingData.AlignmentRadius);
    
    for (APawn* Member : NearbyMembers)
    {
        if (Member && Member != Owner)
        {
            AverageVelocity += Member->GetVelocity();
            Count++;
        }
    }

    if (Count > 0)
    {
        AverageVelocity /= Count;
        AverageVelocity.Normalize();
        AverageVelocity *= FlockingData.MaxSpeed;
        AverageVelocity -= CurrentVelocity;
    }

    return AverageVelocity;
}

FVector UCrowd_FlockingComponent::CalculateCohesion()
{
    APawn* Owner = Cast<APawn>(GetOwner());
    if (!Owner)
    {
        return FVector::ZeroVector;
    }

    FVector OwnerLocation = Owner->GetActorLocation();
    FVector CenterOfMass = FVector::ZeroVector;
    int32 Count = 0;

    TArray<APawn*> NearbyMembers = GetNearbyFlockMembers(FlockingData.CohesionRadius);
    
    for (APawn* Member : NearbyMembers)
    {
        if (Member && Member != Owner)
        {
            CenterOfMass += Member->GetActorLocation();
            Count++;
        }
    }

    if (Count > 0)
    {
        CenterOfMass /= Count;
        FVector ToCenterOfMass = CenterOfMass - OwnerLocation;
        ToCenterOfMass.Normalize();
        ToCenterOfMass *= FlockingData.MaxSpeed;
        ToCenterOfMass -= CurrentVelocity;
        return ToCenterOfMass;
    }

    return FVector::ZeroVector;
}

TArray<APawn*> UCrowd_FlockingComponent::GetNearbyFlockMembers(float Radius)
{
    TArray<APawn*> NearbyMembers;
    APawn* Owner = Cast<APawn>(GetOwner());
    
    if (!Owner)
    {
        return NearbyMembers;
    }

    FVector OwnerLocation = Owner->GetActorLocation();
    
    for (TObjectPtr<APawn> Member : FlockMembers)
    {
        if (Member.IsValid() && Member.Get() != Owner)
        {
            float Distance = FVector::Dist(OwnerLocation, Member->GetActorLocation());
            if (Distance <= Radius)
            {
                NearbyMembers.Add(Member.Get());
            }
        }
    }

    return NearbyMembers;
}