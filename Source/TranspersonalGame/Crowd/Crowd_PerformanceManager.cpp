#include "Crowd_PerformanceManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

void UCrowd_PerformanceManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Initialize default settings
    Settings.MaxCrowdAgents = 5000;
    Settings.TargetFrameRate = 60.0f;
    Settings.LODDistance0 = 500.0f;
    Settings.LODDistance1 = 1500.0f;
    Settings.LODDistance2 = 3000.0f;
    Settings.CullingDistance = 5000.0f;
    Settings.bEnableAdaptiveLOD = true;
    Settings.bEnableOcclusionCulling = true;
    Settings.bEnableFrustumCulling = true;

    // Initialize metrics
    CurrentMetrics = FCrowd_PerformanceMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("Crowd Performance Manager initialized"));
}

void UCrowd_PerformanceManager::Deinitialize()
{
    RegisteredAgents.Empty();
    Super::Deinitialize();
}

void UCrowd_PerformanceManager::UpdatePerformanceMetrics()
{
    if (!GetWorld())
    {
        return;
    }

    // Update frame time
    CurrentMetrics.AverageFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to ms

    // Count active and visible agents
    CurrentMetrics.ActiveCrowdAgents = 0;
    CurrentMetrics.VisibleCrowdAgents = 0;
    CurrentMetrics.LODLevel0Count = 0;
    CurrentMetrics.LODLevel1Count = 0;
    CurrentMetrics.LODLevel2Count = 0;
    CurrentMetrics.CulledAgentsCount = 0;

    // Clean up invalid references
    RegisteredAgents.RemoveAll([](const TWeakObjectPtr<AActor>& WeakPtr)
    {
        return !WeakPtr.IsValid();
    });

    // Update metrics for valid agents
    for (const TWeakObjectPtr<AActor>& WeakAgent : RegisteredAgents)
    {
        if (AActor* Agent = WeakAgent.Get())
        {
            CurrentMetrics.ActiveCrowdAgents++;

            float DistanceToPlayer = GetDistanceToPlayer(Agent);
            
            if (DistanceToPlayer > Settings.CullingDistance)
            {
                CurrentMetrics.CulledAgentsCount++;
                continue;
            }

            if (IsAgentVisible(Agent))
            {
                CurrentMetrics.VisibleCrowdAgents++;
            }

            // Count LOD levels
            int32 LODLevel = GetOptimalLODLevel(DistanceToPlayer);
            switch (LODLevel)
            {
                case 0: CurrentMetrics.LODLevel0Count++; break;
                case 1: CurrentMetrics.LODLevel1Count++; break;
                case 2: CurrentMetrics.LODLevel2Count++; break;
            }
        }
    }

    // Check if emergency optimization is needed
    if (CurrentMetrics.AverageFrameTime > (1000.0f / Settings.TargetFrameRate) * 1.5f)
    {
        if (!bEmergencyOptimizationActive)
        {
            EnableEmergencyOptimization();
        }
    }
    else if (bEmergencyOptimizationActive && 
             CurrentMetrics.AverageFrameTime < (1000.0f / Settings.TargetFrameRate) * 1.2f)
    {
        DisableEmergencyOptimization();
    }
}

void UCrowd_PerformanceManager::SetPerformanceSettings(const FCrowd_PerformanceSettings& NewSettings)
{
    Settings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("Crowd performance settings updated"));
}

void UCrowd_PerformanceManager::OptimizeCrowdSystem()
{
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    if (CurrentTime - LastOptimizationTime < OptimizationInterval)
    {
        return;
    }

    LastOptimizationTime = CurrentTime;

    UpdateLODLevels();
    UpdateCulling();

    if (bEmergencyOptimizationActive)
    {
        PerformEmergencyOptimization();
    }
}

bool UCrowd_PerformanceManager::ShouldSpawnNewAgent() const
{
    if (CurrentMetrics.ActiveCrowdAgents >= Settings.MaxCrowdAgents)
    {
        return false;
    }

    if (bEmergencyOptimizationActive)
    {
        return false;
    }

    if (CurrentMetrics.AverageFrameTime > (1000.0f / Settings.TargetFrameRate) * 1.3f)
    {
        return false;
    }

    return true;
}

int32 UCrowd_PerformanceManager::GetOptimalLODLevel(float Distance) const
{
    if (Distance <= Settings.LODDistance0)
    {
        return 0; // Highest detail
    }
    else if (Distance <= Settings.LODDistance1)
    {
        return 1; // Medium detail
    }
    else if (Distance <= Settings.LODDistance2)
    {
        return 2; // Low detail
    }
    else
    {
        return 3; // Culled
    }
}

void UCrowd_PerformanceManager::RegisterCrowdAgent(AActor* Agent)
{
    if (Agent && !RegisteredAgents.ContainsByPredicate([Agent](const TWeakObjectPtr<AActor>& WeakPtr)
    {
        return WeakPtr.Get() == Agent;
    }))
    {
        RegisteredAgents.Add(Agent);
    }
}

void UCrowd_PerformanceManager::UnregisterCrowdAgent(AActor* Agent)
{
    if (Agent)
    {
        RegisteredAgents.RemoveAll([Agent](const TWeakObjectPtr<AActor>& WeakPtr)
        {
            return WeakPtr.Get() == Agent;
        });
    }
}

void UCrowd_PerformanceManager::EnableEmergencyOptimization()
{
    bEmergencyOptimizationActive = true;
    
    // Reduce LOD distances for emergency optimization
    Settings.LODDistance0 *= 0.7f;
    Settings.LODDistance1 *= 0.7f;
    Settings.LODDistance2 *= 0.7f;
    Settings.CullingDistance *= 0.8f;
    
    UE_LOG(LogTemp, Warning, TEXT("Emergency crowd optimization ENABLED"));
}

void UCrowd_PerformanceManager::DisableEmergencyOptimization()
{
    bEmergencyOptimizationActive = false;
    
    // Restore original LOD distances
    Settings.LODDistance0 /= 0.7f;
    Settings.LODDistance1 /= 0.7f;
    Settings.LODDistance2 /= 0.7f;
    Settings.CullingDistance /= 0.8f;
    
    UE_LOG(LogTemp, Log, TEXT("Emergency crowd optimization DISABLED"));
}

void UCrowd_PerformanceManager::UpdateLODLevels()
{
    if (!Settings.bEnableAdaptiveLOD)
    {
        return;
    }

    for (const TWeakObjectPtr<AActor>& WeakAgent : RegisteredAgents)
    {
        if (AActor* Agent = WeakAgent.Get())
        {
            float Distance = GetDistanceToPlayer(Agent);
            int32 OptimalLOD = GetOptimalLODLevel(Distance);
            
            // Apply LOD level to agent (this would typically involve setting mesh LOD, animation quality, etc.)
            // For now, we'll just set a custom property if the agent supports it
            if (Agent->GetClass()->FindPropertyByName(TEXT("CurrentLODLevel")))
            {
                // Set LOD level property if it exists
            }
        }
    }
}

void UCrowd_PerformanceManager::UpdateCulling()
{
    for (const TWeakObjectPtr<AActor>& WeakAgent : RegisteredAgents)
    {
        if (AActor* Agent = WeakAgent.Get())
        {
            float Distance = GetDistanceToPlayer(Agent);
            bool bShouldBeCulled = Distance > Settings.CullingDistance;
            
            if (Settings.bEnableFrustumCulling)
            {
                bShouldBeCulled = bShouldBeCulled || !IsAgentVisible(Agent);
            }
            
            // Apply culling
            Agent->SetActorHiddenInGame(bShouldBeCulled);
            Agent->SetActorTickEnabled(!bShouldBeCulled);
        }
    }
}

void UCrowd_PerformanceManager::PerformEmergencyOptimization()
{
    // In emergency mode, cull more aggressively
    int32 TargetAgentCount = Settings.MaxCrowdAgents * 0.6f; // Reduce to 60% capacity
    
    if (CurrentMetrics.ActiveCrowdAgents > TargetAgentCount)
    {
        // Sort agents by distance and cull the furthest ones
        TArray<TPair<float, TWeakObjectPtr<AActor>>> AgentDistances;
        
        for (const TWeakObjectPtr<AActor>& WeakAgent : RegisteredAgents)
        {
            if (AActor* Agent = WeakAgent.Get())
            {
                float Distance = GetDistanceToPlayer(Agent);
                AgentDistances.Add(TPair<float, TWeakObjectPtr<AActor>>(Distance, WeakAgent));
            }
        }
        
        // Sort by distance (furthest first)
        AgentDistances.Sort([](const TPair<float, TWeakObjectPtr<AActor>>& A, const TPair<float, TWeakObjectPtr<AActor>>& B)
        {
            return A.Key > B.Key;
        });
        
        // Cull excess agents
        int32 AgentsToCull = CurrentMetrics.ActiveCrowdAgents - TargetAgentCount;
        for (int32 i = 0; i < AgentsToCull && i < AgentDistances.Num(); i++)
        {
            if (AActor* Agent = AgentDistances[i].Value.Get())
            {
                Agent->SetActorHiddenInGame(true);
                Agent->SetActorTickEnabled(false);
            }
        }
    }
}

bool UCrowd_PerformanceManager::IsAgentVisible(AActor* Agent) const
{
    if (!Agent || !GetWorld())
    {
        return false;
    }

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
    {
        return false;
    }

    // Simple frustum check - in a real implementation, you'd use proper frustum culling
    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    FVector AgentLocation = Agent->GetActorLocation();
    FVector PlayerForward = PC->GetPawn()->GetActorForwardVector();
    
    FVector ToAgent = (AgentLocation - PlayerLocation).GetSafeNormal();
    float DotProduct = FVector::DotProduct(PlayerForward, ToAgent);
    
    // Agent is visible if it's in front of the player (simple check)
    return DotProduct > -0.5f; // 120-degree FOV approximation
}

float UCrowd_PerformanceManager::GetDistanceToPlayer(AActor* Agent) const
{
    if (!Agent || !GetWorld())
    {
        return 999999.0f;
    }

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
    {
        return 999999.0f;
    }

    return FVector::Dist(PC->GetPawn()->GetActorLocation(), Agent->GetActorLocation());
}