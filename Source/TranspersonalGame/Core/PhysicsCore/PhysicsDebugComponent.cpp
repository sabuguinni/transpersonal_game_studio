#include "PhysicsDebugComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"

UPhysicsDebugComponent::UPhysicsDebugComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    bShowCollisionShapes = false;
    bShowVelocityVectors = false;
    bShowForceVectors = false;
    bShowContactPoints = false;
    bShowConstraints = false;
    bLogPhysicsEvents = false;
    
    VelocityScale = 1.0f;
    ForceScale = 0.01f;
    DebugLineThickness = 2.0f;
    DebugDuration = 0.0f;
    
    MaxLoggedEvents = 100;
    PhysicsEventLog.Reserve(MaxLoggedEvents);
}

void UPhysicsDebugComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Find the physics system manager
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            if (UPhysicsSystemManager* Manager = ActorItr->FindComponentByClass<UPhysicsSystemManager>())
            {
                PhysicsManager = Manager;
                break;
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsDebugComponent: BeginPlay - Manager found: %s"), 
           PhysicsManager ? TEXT("Yes") : TEXT("No"));
}

void UPhysicsDebugComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GetWorld() || !IsDebugEnabled())
        return;
    
    DrawDebugInfo();
    UpdatePerformanceMetrics(DeltaTime);
}

void UPhysicsDebugComponent::DrawDebugInfo()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    AActor* Owner = GetOwner();
    if (!Owner) return;
    
    // Draw collision shapes
    if (bShowCollisionShapes)
    {
        DrawCollisionShapes(Owner);
    }
    
    // Draw velocity vectors
    if (bShowVelocityVectors)
    {
        DrawVelocityVectors(Owner);
    }
    
    // Draw force vectors
    if (bShowForceVectors)
    {
        DrawForceVectors(Owner);
    }
    
    // Draw contact points
    if (bShowContactPoints)
    {
        DrawContactPoints(Owner);
    }
}

void UPhysicsDebugComponent::DrawCollisionShapes(AActor* Actor)
{
    if (!Actor) return;
    
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Primitive : PrimitiveComponents)
    {
        if (!Primitive || !Primitive->IsCollisionEnabled()) continue;
        
        FVector Location = Primitive->GetComponentLocation();
        FRotator Rotation = Primitive->GetComponentRotation();
        FVector Scale = Primitive->GetComponentScale();
        
        // Draw bounding box
        FBox BoundingBox = Primitive->Bounds.GetBox();
        DrawDebugBox(GetWorld(), BoundingBox.GetCenter(), BoundingBox.GetExtent(), 
                    Rotation.Quaternion(), FColor::Green, false, DebugDuration, 0, DebugLineThickness);
        
        // Draw collision shape based on type
        if (UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(Primitive))
        {
            DrawDebugSphere(GetWorld(), Location, 50.0f, 12, FColor::Blue, false, DebugDuration, 0, DebugLineThickness);
        }
    }
}

void UPhysicsDebugComponent::DrawVelocityVectors(AActor* Actor)
{
    if (!Actor) return;
    
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Primitive : PrimitiveComponents)
    {
        if (!Primitive || !Primitive->IsSimulatingPhysics()) continue;
        
        FVector Location = Primitive->GetComponentLocation();
        FVector Velocity = Primitive->GetPhysicsLinearVelocity();
        
        if (!Velocity.IsNearlyZero())
        {
            FVector EndPoint = Location + (Velocity * VelocityScale);
            DrawDebugDirectionalArrow(GetWorld(), Location, EndPoint, 20.0f, 
                                    FColor::Yellow, false, DebugDuration, 0, DebugLineThickness);
            
            // Draw velocity magnitude as text
            FString VelText = FString::Printf(TEXT("Vel: %.1f"), Velocity.Size());
            DrawDebugString(GetWorld(), EndPoint, VelText, nullptr, FColor::Yellow, DebugDuration);
        }
    }
}

void UPhysicsDebugComponent::DrawForceVectors(AActor* Actor)
{
    if (!Actor) return;
    
    // This would require integration with the physics system to track applied forces
    // For now, we'll draw placeholder force vectors
    FVector Location = Actor->GetActorLocation();
    
    // Example: Draw gravity force
    FVector GravityForce = FVector(0, 0, -980.0f) * ForceScale;
    if (!GravityForce.IsNearlyZero())
    {
        FVector EndPoint = Location + GravityForce;
        DrawDebugDirectionalArrow(GetWorld(), Location, EndPoint, 15.0f, 
                                FColor::Red, false, DebugDuration, 0, DebugLineThickness);
        
        FString ForceText = FString::Printf(TEXT("Gravity: %.1f"), GravityForce.Size());
        DrawDebugString(GetWorld(), EndPoint, ForceText, nullptr, FColor::Red, DebugDuration);
    }
}

void UPhysicsDebugComponent::DrawContactPoints(AActor* Actor)
{
    // Contact point visualization would require integration with Chaos physics callbacks
    // This is a placeholder implementation
    if (!Actor) return;
    
    FVector Location = Actor->GetActorLocation();
    DrawDebugSphere(GetWorld(), Location, 5.0f, 8, FColor::Magenta, false, DebugDuration, 0, DebugLineThickness);
}

void UPhysicsDebugComponent::UpdatePerformanceMetrics(float DeltaTime)
{
    CurrentFrameTime = DeltaTime;
    
    // Update running averages
    static float AccumulatedTime = 0.0f;
    static int32 FrameCount = 0;
    
    AccumulatedTime += DeltaTime;
    FrameCount++;
    
    if (AccumulatedTime >= 1.0f) // Update every second
    {
        AverageFrameTime = AccumulatedTime / FrameCount;
        AccumulatedTime = 0.0f;
        FrameCount = 0;
        
        // Log performance if enabled
        if (bLogPhysicsEvents)
        {
            LogPhysicsEvent(FString::Printf(TEXT("Performance: Avg Frame Time: %.3fms, FPS: %.1f"), 
                                          AverageFrameTime * 1000.0f, 1.0f / AverageFrameTime));
        }
    }
}

void UPhysicsDebugComponent::LogPhysicsEvent(const FString& Event)
{
    if (!bLogPhysicsEvents) return;
    
    FDateTime Now = FDateTime::Now();
    FString TimestampedEvent = FString::Printf(TEXT("[%s] %s"), *Now.ToString(), *Event);
    
    PhysicsEventLog.Add(TimestampedEvent);
    
    // Maintain log size limit
    if (PhysicsEventLog.Num() > MaxLoggedEvents)
    {
        PhysicsEventLog.RemoveAt(0);
    }
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsDebug: %s"), *TimestampedEvent);
}

void UPhysicsDebugComponent::ToggleCollisionShapes()
{
    bShowCollisionShapes = !bShowCollisionShapes;
    UE_LOG(LogTemp, Log, TEXT("PhysicsDebug: Collision shapes display: %s"), 
           bShowCollisionShapes ? TEXT("ON") : TEXT("OFF"));
}

void UPhysicsDebugComponent::ToggleVelocityVectors()
{
    bShowVelocityVectors = !bShowVelocityVectors;
    UE_LOG(LogTemp, Log, TEXT("PhysicsDebug: Velocity vectors display: %s"), 
           bShowVelocityVectors ? TEXT("ON") : TEXT("OFF"));
}

void UPhysicsDebugComponent::ToggleForceVectors()
{
    bShowForceVectors = !bShowForceVectors;
    UE_LOG(LogTemp, Log, TEXT("PhysicsDebug: Force vectors display: %s"), 
           bShowForceVectors ? TEXT("ON") : TEXT("OFF"));
}

void UPhysicsDebugComponent::ClearEventLog()
{
    PhysicsEventLog.Empty();
    UE_LOG(LogTemp, Log, TEXT("PhysicsDebug: Event log cleared"));
}

bool UPhysicsDebugComponent::IsDebugEnabled() const
{
    return bShowCollisionShapes || bShowVelocityVectors || bShowForceVectors || 
           bShowContactPoints || bShowConstraints;
}

TArray<FString> UPhysicsDebugComponent::GetEventLog() const
{
    return PhysicsEventLog;
}

FPhysicsPerformanceMetrics UPhysicsDebugComponent::GetPerformanceMetrics() const
{
    FPhysicsPerformanceMetrics Metrics;
    Metrics.CurrentFrameTime = CurrentFrameTime;
    Metrics.AverageFrameTime = AverageFrameTime;
    Metrics.CurrentFPS = 1.0f / FMath::Max(CurrentFrameTime, 0.001f);
    Metrics.AverageFPS = 1.0f / FMath::Max(AverageFrameTime, 0.001f);
    
    return Metrics;
}